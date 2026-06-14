#include "StatisticsUI.h"
#include "UILayout.h"
#include <imgui.h>

void StatisticsUI::render() {
    UILayout::placeStatistics();
    ImGui::Begin("Statistics");

    ImGui::Text("Current tick: %d", m_stats.currentTick);
    ImGui::Text("Finished products: %d", m_stats.finishedProducts);
    ImGui::Text("Work-in-progress (WIP): %d", m_stats.totalPipelineLoad);
    ImGui::Text("Total breakdowns: %d", m_stats.totalBreakdowns);
    ImGui::Text("Lost products (overflow): %d", m_stats.lostProducts);

    ImGui::Separator();
    ImGui::Text("Working machines: %d", m_stats.workingMachines);
    ImGui::Text("Blocked machines: %d", m_stats.blockedMachines);
    ImGui::Text("Broken machines: %d", m_stats.brokenMachines);

    ImGui::End();
}
