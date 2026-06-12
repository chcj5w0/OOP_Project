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

// Color per product type, used for the dots travelling through connectors.
inline ImVec4 productColor(const std::string& productName) {
    if (productName == "RawFeed")         return ImVec4(0.62f, 0.46f, 0.26f, 1.0f); // crude brown
    if (productName == "Intermediate")    return ImVec4(0.93f, 0.66f, 0.22f, 1.0f); // amber
    if (productName == "FinishedProduct") return ImVec4(0.35f, 0.85f, 0.42f, 1.0f); // green
    return ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
}

// Green→yellow→red gradient for health gauges.
inline ImVec4 healthColor(float health) {
    if (health < 0.0f) health = 0.0f;
    if (health > 1.0f) health = 1.0f;
    float r = health < 0.5f ? 1.0f : 2.0f * (1.0f - health);
    float g = health > 0.5f ? 1.0f : 2.0f * health;
    return ImVec4(r * 0.9f, g * 0.8f, 0.15f, 1.0f);
}
