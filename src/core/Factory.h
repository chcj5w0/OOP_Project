#pragma once

#include "SimObject.h"
#include "bridge/FactoryStats.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include <memory>
#include <vector>

class TankTerminal;

class Factory {
public:
    void tick();
    void applyCmd(const MachineCmd& cmd);
    std::vector<MachineSnap> snapshotAll() const;
    FactoryStats stats() const;
    int  currentTick() const { return m_tick; }

    void buildScenarioNormal();

private:
    std::vector<std::unique_ptr<SimObject>> m_objects;
    TankTerminal* m_terminal = nullptr;
    int m_tick = 0;
};
