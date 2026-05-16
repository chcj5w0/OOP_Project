// MachineUI.cpp
#include "MachineUI.h"
#include "imgui.h"

void MachineUI::render() {
    ImGui::Begin("Machine Control", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // ── Status label ────────────────────────────────────────────────────────
    ImGui::Text("Status:");
    ImGui::SameLine();

    bool isActive = (m_ctrl.getStatus() == Status::ACTIVE);

    // Color the label green when ACTIVE, red when STOPPED
    ImVec4 color = isActive
        ? ImVec4(0.2f, 0.8f, 0.2f, 1.0f)   // green
        : ImVec4(0.9f, 0.2f, 0.2f, 1.0f);  // red

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(m_ctrl.getStatusLabel());
    ImGui::PopStyleColor();

    ImGui::Separator();

    // ── Toggle button ────────────────────────────────────────────────────────
    const char* btnLabel = isActive ? "Stop Machine" : "Start Machine";

    if (isActive) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
    }

    if (ImGui::Button(btnLabel, ImVec2(140, 36))) {
        m_ctrl.onToggleClicked();   // UI only calls the controller — never Machine directly
    }

    ImGui::PopStyleColor(3);

    ImGui::End();
}