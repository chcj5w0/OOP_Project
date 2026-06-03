#include "Factory.h"
#include "Connector.h"
#include "Machine.h"
#include "Pipeline.h"
#include "Conveyor.h"
#include "machines/TankTerminal.h"
#include "machines/SourceTank.h"
#include "machines/Reactor.h"
#include "machines/Separator.h"

void Factory::tick() {
    for (auto& o : m_objects) {
        o->update(m_tick);
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
            s.totalPipelineLoad += c->size();
        }
    }

    return s;
}

void Factory::buildScenarioNormal() {
    m_objects.clear();
    m_tick = 0;
    m_terminal = nullptr;

    auto src   = std::make_unique<SourceTank>(/*id*/1, /*emitEvery*/3);
    auto pipe1 = std::make_unique<Pipeline>  (/*id*/2, /*capacity*/8);
    auto rx    = std::make_unique<Reactor>   (/*id*/3, /*processTicks*/4);
    auto pipe2 = std::make_unique<Pipeline>  (/*id*/4, /*capacity*/8);
    auto sep   = std::make_unique<Separator> (/*id*/5, /*processTicks*/3);
    auto pipe3 = std::make_unique<Conveyor>  (/*id*/6, /*capacity*/8);
    auto tank = std::make_unique<TankTerminal>(/*id*/7, /*processTicks*/1);

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
