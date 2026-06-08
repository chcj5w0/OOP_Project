#pragma once

#include "SimObject.h"
#include "bridge/FactoryStats.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include <memory>
#include <vector>

class TankTerminal;
class EventLogUI;

class Factory {
public:
    void tick();
    void applyCmd(const MachineCmd& cmd);
    std::vector<MachineSnap> snapshotAll() const;
    FactoryStats stats() const;
    int  currentTick() const { return m_tick; }

    void buildScenarioNormal();
    void setEventLogUI(EventLogUI* ui) { m_eventLogUI = ui; }

private:
    std::vector<std::unique_ptr<SimObject>> m_objects;
    std::vector<MachineState> m_machineStatesBefore;
    std::vector<float> m_machineHealthBefore;
    int m_prevTankTerminalFinished = 0;
    TankTerminal* m_terminal = nullptr;
    EventLogUI* m_eventLogUI = nullptr;
    int m_tick = 0;
};
