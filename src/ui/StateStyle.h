#pragma once
#include "bridge/MachineSnap.h"
#include <imgui.h>

// Shared label/color mapping for MachineState, used by every view that
// displays machine status. Keep all state→style decisions in this file so
// the views never disagree on what a state looks like.

inline const char* stateLabel(MachineState state) {
    switch (state) {
        case MachineState::IDLE:    return "IDLE";
        case MachineState::WORKING: return "WORKING";
        case MachineState::BLOCKED: return "BLOCKED";
        case MachineState::BROKEN:  return "BROKEN";
    }
    return "UNKNOWN";
}

inline const char* stateLabel(const MachineSnap& snap) {
    if (snap.state == MachineState::BROKEN && snap.repairCountdown > 0) {
        return "BROKEN (repairing)";
    }
    return stateLabel(snap.state);
}

inline ImVec4 stateColor(MachineState state) {
    switch (state) {
        case MachineState::IDLE:    return ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        case MachineState::WORKING: return ImVec4(0.30f, 0.75f, 0.95f, 1.0f);
        case MachineState::BLOCKED: return ImVec4(0.95f, 0.70f, 0.25f, 1.0f);
        case MachineState::BROKEN:  return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

inline ImVec4 stateColor(const MachineSnap& snap) {
    return stateColor(snap.state);
}
