#include "InspectorUI.h"
#include <imgui.h>

void InspectorUI::render(const MachineSnap& snap) {
    ImGui::Begin("Inspector");
    ImGui::Text("ID: %d", snap.id);
    ImGui::Text("Type: %s", snap.typeName);
    ImGui::Text("Health: %.2f", snap.health);
    ImGui::Text("Progress: %d/%d", snap.progress, snap.processTicks);
    ImGui::Text("Input load: %d", snap.inputLoad);
    ImGui::End();
}
