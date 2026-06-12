#pragma once

#include <string>

enum class MachineState { IDLE, WORKING, BLOCKED, BROKEN };

struct MachineSnap {
    int          id;
    const char*  typeName;
    MachineState state;
    float        health;
    int          progress;
    int          processTicks;
    int          inputLoad;
    int          inputCapacity;
    int          outputCount;
    int          outputCapacity;
    int          producedCount;
    int          repairCountdown;
    int          inputId;   // id of the attached input connector, -1 if none
    int          outputId;  // id of the attached output connector, -1 if none
    std::string  blockedReason;
};
