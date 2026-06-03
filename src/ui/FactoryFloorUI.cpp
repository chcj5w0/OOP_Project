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

void FactoryFloorUI::render() {
    ImGui::Begin("Factory Floor");
    ImGui::TextDisabled("No machine snapshot data available.");
    ImGui::End();
}

void FactoryFloorUI::render(const std::vector<MachineSnap>& snaps) {
    ImGui::Begin("Factory Floor");
    for (const auto& snap : snaps) {
        ImGui::Text("ID %d (%s): %s", snap.id, snap.typeName, stateLabel(snap.state));
    }
    ImGui::End();
}
