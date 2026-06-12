#include "SimControlUI.h"
#include <imgui.h>

void SimControlUI::render() {
    ImGui::Begin("Simulation Control");
    ImGui::Checkbox("Start Work", &m_cmd.startWork);
    ImGui::Checkbox("Force Break", &m_cmd.forceBreak);
    ImGui::Checkbox("Instant Repair", &m_cmd.instantRepair);
    ImGui::InputInt("Target ID", &m_cmd.targetId);
    ImGui::End();
}
