#include "InspectorUI.h"
#include <imgui.h>

static const char* stateLabel(MachineState state) {
    switch (state) {
        case MachineState::IDLE:    return "IDLE";
        case MachineState::WORKING: return "WORKING";
        case MachineState::BLOCKED: return "BLOCKED";
        case MachineState::BROKEN:  return "BROKEN";
    }
    return "UNKNOWN";
}

static ImVec4 stateColor(MachineState state) {
    switch (state) {
        case MachineState::IDLE:    return ImVec4(0.72f, 0.72f, 0.72f, 1.0f);
        case MachineState::WORKING: return ImVec4(0.30f, 0.75f, 0.95f, 1.0f);
        case MachineState::BLOCKED: return ImVec4(0.95f, 0.70f, 0.25f, 1.0f);
        case MachineState::BROKEN:  return ImVec4(0.95f, 0.25f, 0.25f, 1.0f);
    }
    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

static float workProgressRatio(const MachineSnap& snap) {
    if (snap.processTicks <= 0) return 0.0f;
    return static_cast<float>(snap.progress) / static_cast<float>(snap.processTicks);
}

void InspectorUI::render(const std::vector<MachineSnap>& snaps) {
    ImGui::Begin("Inspector");

    if (snaps.empty()) {
        ImGui::Text("No machines");
        ImGui::End();
        return;
    }

    constexpr ImGuiTableFlags flags =
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("InspectorTable", 8, flags)) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("State");
        ImGui::TableSetupColumn("Health");
        ImGui::TableSetupColumn("Work");
        ImGui::TableSetupColumn("Input");
        ImGui::TableSetupColumn("Output");
        ImGui::TableSetupColumn("Process");
        ImGui::TableHeadersRow();

        for (const auto& snap : snaps) {
            ImGui::PushID(snap.id);
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", snap.id);

            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(snap.typeName);

            ImGui::TableSetColumnIndex(2);
            ImGui::TextColored(stateColor(snap.state), "%s", stateLabel(snap.state));

            ImGui::TableSetColumnIndex(3);
            ImGui::ProgressBar(snap.health, ImVec2(-1.0f, 0.0f), "");
            ImGui::SameLine();
            ImGui::Text("%.0f%%", snap.health * 100.0f);

            ImGui::TableSetColumnIndex(4);
            ImGui::ProgressBar(workProgressRatio(snap), ImVec2(-1.0f, 0.0f), "");
            ImGui::SameLine();
            ImGui::Text("%d/%d", snap.progress, snap.processTicks);

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%d", snap.inputLoad);

            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%d", snap.outputCount);

            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%d ticks", snap.processTicks);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
