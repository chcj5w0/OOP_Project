#include "StatisticsUI.h"
#include <imgui.h>

void StatisticsUI::render() {
    ImGui::Begin("Statistics");

    ImGui::Text("Current tick: %d", m_stats.currentTick);
    ImGui::Text("Finished products: %d", m_stats.finishedProducts);

    ImGui::Separator();
    ImGui::Text("Working machines: %d", m_stats.workingMachines);
    ImGui::Text("Blocked machines: %d", m_stats.blockedMachines);
    ImGui::Text("Broken machines: %d", m_stats.brokenMachines);

    ImGui::Separator();
    ImGui::Text("Total pipeline load: %d", m_stats.totalPipelineLoad);

    ImGui::End();
}
