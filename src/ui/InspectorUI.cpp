#include "InspectorUI.h"
#include "StateStyle.h"
#include <imgui.h>

static float workProgressRatio(const MachineSnap& snap) {
    if (snap.processTicks <= 0) return 0.0f;
    return static_cast<float>(snap.progress) / static_cast<float>(snap.processTicks);
}

void InspectorUI::render() {
    ImGui::Begin("Inspector");

    if (m_snaps.empty()) {
        ImGui::Text("No machines");
        ImGui::End();
        return;
    }

    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("InspectorTable", 10, flags)) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("Health");
        ImGui::TableSetupColumn("Work");
        ImGui::TableSetupColumn("Input");
        ImGui::TableSetupColumn("Output");
        ImGui::TableSetupColumn("Produced");
        ImGui::TableSetupColumn("Repair");
        ImGui::TableSetupColumn("Reason");
        ImGui::TableHeadersRow();

        for (const auto& snap : m_snaps) {
            ImGui::PushID(snap.id);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", snap.id);

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(snap.typeName);

            ImGui::TableSetColumnIndex(2);
            ImGui::TextColored(stateColor(snap), "%s", stateLabel(snap));

            ImGui::TableSetColumnIndex(3);
            ImGui::ProgressBar(snap.health, ImVec2(-1.0f, 0.0f), "");
            ImGui::SameLine();
            ImGui::Text("%.0f%%", snap.health * 100.0f);

            ImGui::TableSetColumnIndex(4);
            ImGui::ProgressBar(workProgressRatio(snap), ImVec2(-1.0f, 0.0f), "");
            ImGui::SameLine();
            ImGui::Text("%d/%d", snap.progress, snap.processTicks);

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%d/%d", snap.inputLoad, snap.inputCapacity);

            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%d/%d", snap.outputCount, snap.outputCapacity);

            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%d", snap.producedCount);

            ImGui::TableSetColumnIndex(8);
            if (snap.repairCountdown > 0) {
                ImGui::Text("%d ticks", snap.repairCountdown);
            } else {
                ImGui::TextUnformatted("-");
            }

            ImGui::TableSetColumnIndex(9);
            ImGui::TextUnformatted(snap.blockedReason.empty() ? "-" : snap.blockedReason.c_str());

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
