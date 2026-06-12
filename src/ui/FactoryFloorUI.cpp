#include "FactoryFloorUI.h"
#include "StateStyle.h"
#include <imgui.h>

void FactoryFloorUI::render() {
    ImGui::Begin("Factory Floor");
    if (m_snaps.empty()) {
        ImGui::TextDisabled("No machine snapshot data available.");
        ImGui::End();
        return;
    }
    for (const auto& snap : m_snaps) {
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
