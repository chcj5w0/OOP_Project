#pragma once

struct FactoryStats {
    int currentTick       = 0;
    int finishedProducts  = 0;
    int workingMachines   = 0;
    int blockedMachines   = 0;
    int brokenMachines    = 0;
    int totalPipelineLoad = 0;   // products currently in connectors = WIP
    int totalBreakdowns   = 0;   // cumulative breakdown count
    int lostProducts      = 0;   // cumulative products lost to overflow
};
