#pragma once

enum class MachineState { IDLE, WORKING, BLOCKED, BROKEN };

struct MachineSnap {
    int          id;
    const char*  typeName;
    MachineState state;
    float        health;
    int          progress;
    int          processTicks;
    int          outputCount;
    int          inputLoad;
};
