#pragma once

#include "SimObject.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include <memory>
#include <vector>

class Factory {
public:
    void tick();
    void applyCmd(const MachineCmd& cmd);
    std::vector<MachineSnap> snapshotAll() const;
    int  currentTick() const { return m_tick; }

    void buildScenarioNormal();

private:
    std::vector<std::unique_ptr<SimObject>> m_objects;
    int m_tick = 0;
};
