#pragma once

#include "SimObject.h"
#include "bridge/ConnectorSnap.h"
#include "bridge/FactoryStats.h"
#include "bridge/IEventObserver.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include <memory>
#include <vector>

class TankTerminal;
class Scenario;

// Tunable knobs that distinguish one scenario from another. Public so that
// Scenario objects (which live outside Factory) can describe a pipeline and
// hand it to Factory::build().
struct ScenarioParams {
    int   emitInterval  = 3;   // SourceTank: a RawFeed every N ticks
    int   reactorTicks  = 4;
    int   separatorTicks= 3;
    int   terminalTicks = 1;
    int   pipeCapacity  = 8;
    int   convCapacity  = 8;
    float breakdownProb = 0.0f;
    bool  dropOnOverflow= false;
};

class Factory {
public:
    void tick();
    void applyCmd(const MachineCmd& cmd);
    std::vector<MachineSnap> snapshotAll() const;
    std::vector<ConnectorSnap> snapshotConnectors() const;
    FactoryStats stats() const;
    int  currentTick() const { return m_tick; }

    // Scenario selection (§2.1). The Factory holds no switch over scenario
    // types — it just asks the chosen Scenario to configure it (polymorphism).
    void loadScenario(const Scenario& scenario);
    void reset();                                            // rebuild current scenario
    const Scenario* currentScenario() const { return m_currentScenario; }

    // Called by Scenario::configure() to assemble the pipeline.
    void build(const ScenarioParams& p);

    // Observer registration. Factory notifies observers of events without
    // knowing their concrete type (Observer pattern).
    void addObserver(IEventObserver* obs) { if (obs) m_observers.push_back(obs); }

private:
    int  totalLostProducts() const;   // summed across all machines
    void emitEvent(const char* fmt, ...);   // format + notify all observers

    std::vector<std::unique_ptr<SimObject>> m_objects;
    std::vector<MachineState> m_machineStatesBefore;
    std::vector<float> m_machineHealthBefore;
    int m_prevTankTerminalFinished = 0;
    int m_prevLostProducts = 0;
    int m_totalBreakdowns = 0;
    const Scenario* m_currentScenario = nullptr;
    TankTerminal* m_terminal = nullptr;
    std::vector<IEventObserver*> m_observers;
    int m_tick = 0;
};
