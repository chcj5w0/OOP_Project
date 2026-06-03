#include "StatisticsUI.h"
#include <imgui.h>

void StatisticsUI::render(const FactoryStats& stats) {
    ImGui::Begin("Statistics");

    ImGui::Text("Current tick: %d", stats.currentTick);
    ImGui::Text("Finished products: %d", stats.finishedProducts);

    ImGui::Separator();
    ImGui::Text("Working machines: %d", stats.workingMachines);
    ImGui::Text("Blocked machines: %d", stats.blockedMachines);
    ImGui::Text("Broken machines: %d", stats.brokenMachines);

    ImGui::Separator();
    ImGui::Text("Total pipeline load: %d", stats.totalPipelineLoad);

    ImGui::End();
}
