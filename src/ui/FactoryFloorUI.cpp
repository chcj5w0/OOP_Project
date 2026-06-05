#include "FactoryFloorUI.h"
#include <imgui.h>

static const char* stateLabel(MachineState s) {
    switch (s) {
        case MachineState::IDLE:    return "IDLE";
        case MachineState::WORKING: return "WORKING";
        case MachineState::BLOCKED: return "BLOCKED";
        case MachineState::BROKEN:  return "BROKEN";
    }
    return "?";
}

static const char* stateLabel(const MachineSnap& snap) {
    if (snap.state == MachineState::BROKEN && snap.repairCountdown > 0) {
        return "BROKEN (repairing)";
    }
    return stateLabel(snap.state);
}

static ImVec4 stateColor(MachineState state) {
    switch (state) {
        case MachineState::IDLE:    return ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        case MachineState::WORKING: return ImVec4(0.30f, 0.75f, 0.95f, 1.0f);
        case MachineState::BLOCKED: return ImVec4(0.95f, 0.70f, 0.25f, 1.0f);
        case MachineState::BROKEN:  return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

static ImVec4 stateColor(const MachineSnap& snap) {
    if (snap.state == MachineState::BROKEN && snap.repairCountdown > 0) {
        return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    return stateColor(snap.state);
}

void FactoryFloorUI::render() {
    ImGui::Begin("Factory Floor");
    ImGui::TextDisabled("No machine snapshot data available.");
    ImGui::End();
}

void FactoryFloorUI::render(const std::vector<MachineSnap>& snaps) {
    ImGui::Begin("Factory Floor");
    for (const auto& snap : snaps) {
        const char* reason = snap.blockedReason.empty() ? "-" : snap.blockedReason.c_str();
        if (snap.repairCountdown > 0) {
            ImGui::TextColored(stateColor(snap), "ID %d (%s): %s | in %d/%d out %d/%d prod %d | repair in %d ticks | %s",
                        snap.id, snap.typeName, stateLabel(snap), snap.inputLoad,
                        snap.inputCapacity, snap.outputCount, snap.outputCapacity,
                        snap.producedCount, snap.repairCountdown, reason);
        } else {
            ImGui::TextColored(stateColor(snap), "ID %d (%s): %s | in %d/%d out %d/%d prod %d | %s",
                        snap.id, snap.typeName, stateLabel(snap), snap.inputLoad,
                        snap.inputCapacity, snap.outputCount, snap.outputCapacity,
                        snap.producedCount, reason);
        }
    }
    ImGui::End();
}
