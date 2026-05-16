#include "SimControlUI.h"
#include <imgui.h>

void SimControlUI::render(MachineCmd& cmd) {
    ImGui::Begin("Simulation Control");
    ImGui::Checkbox("Start Work", &cmd.startWork);
    ImGui::Checkbox("Force Break", &cmd.forceBreak);
    ImGui::Checkbox("Instant Repair", &cmd.instantRepair);
    ImGui::InputInt("Target ID", &cmd.targetId);
    ImGui::End();
}
