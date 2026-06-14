#include "Factory.h"
#include "Scenario.h"
#include "Connector.h"
#include "Machine.h"
#include "Pipeline.h"
#include "Conveyor.h"
#include "machines/TankTerminal.h"
#include "machines/SourceTank.h"
#include "machines/Reactor.h"
#include "machines/Separator.h"
#include <cstdarg>
#include <cstdio>

void Factory::emitEvent(const char* fmt, ...) {
    if (m_observers.empty()) return;
    char msg[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);
    for (IEventObserver* obs : m_observers) {
        obs->onEvent(m_tick, msg);
    }
}

void Factory::tick() {
    // Save machine states and health before update
    m_machineStatesBefore.clear();
    m_machineHealthBefore.clear();
    for (const auto& o : m_objects) {
        if (auto* m = dynamic_cast<Machine*>(o.get())) {
            m_machineStatesBefore.push_back(m->state());
            m_machineHealthBefore.push_back(m->health());
        }
    }
    
    // Save counters that we diff after the update to emit "delta" events.
    if (m_terminal) {
        m_prevTankTerminalFinished = m_terminal->finishedProducts();
    }
    m_prevLostProducts = totalLostProducts();

    // Update all objects
    for (auto& o : m_objects) {
        o->update(m_tick);
    }
    
    // Check for selective events
    int machineIdx = 0;
    for (const auto& o : m_objects) {
        if (auto* m = dynamic_cast<Machine*>(o.get())) {
            MachineState oldState = m_machineStatesBefore[machineIdx];
            MachineState newState = m->state();
            float oldHealth = m_machineHealthBefore[machineIdx];
            float newHealth = m->health();
            
            if (oldState != MachineState::BROKEN && newState == MachineState::BROKEN) {
                ++m_totalBreakdowns;
                emitEvent("Machine %d BROKEN", m->id());
            }
            if (oldState == MachineState::BROKEN && newState == MachineState::IDLE) {
                emitEvent("Machine %d REPAIRED", m->id());
            }
            if (oldState != MachineState::BLOCKED && newState == MachineState::BLOCKED) {
                emitEvent("Machine %d BLOCKED", m->id());
            }
            if (oldState != MachineState::WORKING && newState == MachineState::WORKING) {
                emitEvent("Machine %d WORKING", m->id());
            }
            if (oldHealth > 0.3f && newHealth <= 0.3f && newHealth > 0.0f) {
                emitEvent("Machine %d HEALTH CRITICAL (%.2f)", m->id(), newHealth);
            }

            ++machineIdx;
        }
    }
    
    // Check TankTerminal finished products
    if (m_terminal && m_terminal->finishedProducts() > m_prevTankTerminalFinished) {
        emitEvent("Product finished! Total: %d", m_terminal->finishedProducts());
    }

    // Overflow: products dropped because a buffer was full (§2.1 Overflow).
    int lostNow = totalLostProducts();
    if (lostNow > m_prevLostProducts) {
        emitEvent("Product LOST to overflow! Total lost: %d", lostNow);
    }

    ++m_tick;
}

void Factory::applyCmd(const MachineCmd& cmd) {
    for (auto& o : m_objects) {
        if (auto* m = dynamic_cast<Machine*>(o.get())) {
            m->applyCmd(cmd);
        }
    }
}

std::vector<MachineSnap> Factory::snapshotAll() const {
    std::vector<MachineSnap> snaps;
    for (const auto& o : m_objects) {
        if (auto* m = dynamic_cast<const Machine*>(o.get())) {
            snaps.push_back(m->snapshot());
        }
    }
    return snaps;
}

std::vector<ConnectorSnap> Factory::snapshotConnectors() const {
    std::vector<ConnectorSnap> snaps;
    for (const auto& o : m_objects) {
        if (auto* c = dynamic_cast<const Connector*>(o.get())) {
            snaps.push_back(c->snapshot());
        }
    }
    return snaps;
}

FactoryStats Factory::stats() const {
    FactoryStats s{};
    s.currentTick = m_tick;
    s.finishedProducts = m_terminal ? m_terminal->finishedProducts() : 0;

    for (const auto& o : m_objects) {
        if (auto* m = dynamic_cast<const Machine*>(o.get())) {
            switch (m->state()) {
                case MachineState::WORKING:
                    ++s.workingMachines;
                    break;
                case MachineState::BLOCKED:
                    ++s.blockedMachines;
                    break;
                case MachineState::BROKEN:
                    ++s.brokenMachines;
                    break;
                case MachineState::IDLE:
                    break;
            }
        } else if (auto* c = dynamic_cast<const Connector*>(o.get())) {
            s.totalPipelineLoad += c->size();   // products in transit = WIP
        }
    }

    s.totalBreakdowns = m_totalBreakdowns;
    s.lostProducts    = totalLostProducts();
    return s;
}

int Factory::totalLostProducts() const {
    int lost = 0;
    for (const auto& o : m_objects) {
        if (auto* m = dynamic_cast<const Machine*>(o.get())) {
            lost += m->lostProducts();
        }
    }
    return lost;
}

void Factory::loadScenario(const Scenario& scenario) {
    m_currentScenario = &scenario;
    scenario.configure(*this);   // the scenario decides how to build us
}

void Factory::reset() {
    if (m_currentScenario) loadScenario(*m_currentScenario);
}

void Factory::build(const ScenarioParams& p) {
    m_objects.clear();
    m_tick = 0;
    m_terminal = nullptr;
    m_prevTankTerminalFinished = 0;
    m_prevLostProducts = 0;
    m_totalBreakdowns = 0;
    m_machineStatesBefore.clear();
    m_machineHealthBefore.clear();

    auto src   = std::make_unique<SourceTank>(/*id*/1, p.emitInterval);
    auto pipe1 = std::make_unique<Pipeline>  (/*id*/2, p.pipeCapacity);
    auto rx    = std::make_unique<Reactor>   (/*id*/3, p.reactorTicks);
    auto pipe2 = std::make_unique<Pipeline>  (/*id*/4, p.pipeCapacity);
    auto sep   = std::make_unique<Separator> (/*id*/5, p.separatorTicks);
    auto pipe3 = std::make_unique<Conveyor>  (/*id*/6, p.convCapacity);
    auto tank  = std::make_unique<TankTerminal>(/*id*/7, p.terminalTicks);

    // Apply per-scenario tuning uniformly to every machine.
    for (Machine* m : { static_cast<Machine*>(src.get()), static_cast<Machine*>(rx.get()),
                        static_cast<Machine*>(sep.get()), static_cast<Machine*>(tank.get()) }) {
        m->setBreakdownProb(p.breakdownProb);
        m->setDropOnOverflow(p.dropOnOverflow);
    }

    src->attachOutput(pipe1.get());
    rx ->attachInput (pipe1.get());
    rx ->attachOutput(pipe2.get());
    sep->attachInput (pipe2.get());
    sep->attachOutput(pipe3.get());
    tank->attachInput(pipe3.get());
    m_terminal = tank.get();

    m_objects.push_back(std::move(src));
    m_objects.push_back(std::move(pipe1));
    m_objects.push_back(std::move(rx));
    m_objects.push_back(std::move(pipe2));
    m_objects.push_back(std::move(sep));
    m_objects.push_back(std::move(pipe3));
    m_objects.push_back(std::move(tank));
}
