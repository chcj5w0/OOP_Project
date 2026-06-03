#pragma once

#include "SimObject.h"
#include "bridge/FactoryStats.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include <memory>
#include <vector>

class Pipeline;

class Factory {
public:
    void tick();
    void applyCmd(const MachineCmd& cmd);
    std::vector<MachineSnap> snapshotAll() const;
    FactoryStats stats() const;
    int  currentTick() const { return m_tick; }

    void buildScenarioNormal();

private:
    void collectFinishedProducts();

    std::vector<std::unique_ptr<SimObject>> m_objects;
    Pipeline* m_finalOutput = nullptr;
    int m_tick = 0;
    int m_finishedProducts = 0;
};
