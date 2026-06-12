#include "SimControlUI.h"
#include "StateStyle.h"
#include "UILayout.h"
#include <imgui.h>
#include <cstdio>

void SimControlUI::render() {
    UILayout::placeSimControl();
    ImGui::Begin("Simulation Control");

    // Target picker: combo over the machines in the current snapshot.
    const MachineSnap* target = nullptr;
    for (const auto& snap : m_snaps) {
        if (snap.id == m_cmd.targetId) { target = &snap; break; }
    }

    char preview[64];
    if (target) {
        snprintf(preview, sizeof(preview), "%s [%d]", target->typeName, target->id);
    } else {
        snprintf(preview, sizeof(preview), "(select machine)");
    }

    if (ImGui::BeginCombo("Target", preview)) {
        for (const auto& snap : m_snaps) {
            char label[64];
            snprintf(label, sizeof(label), "%s [%d]", snap.typeName, snap.id);
            if (ImGui::Selectable(label, snap.id == m_cmd.targetId)) {
                m_cmd.targetId = snap.id;
            }
        }
        ImGui::EndCombo();
    }

    if (target) {
        ImGui::TextColored(stateColor(*target), "state: %s", stateLabel(*target));
    }

    ImGui::Separator();

    // One-shot commands; the main loop consumes and clears these flags.
    ImGui::BeginDisabled(target == nullptr);
    if (ImGui::Button("Start Work"))     m_cmd.startWork     = true;
    ImGui::SameLine();
    if (ImGui::Button("Force Break"))    m_cmd.forceBreak    = true;
    ImGui::SameLine();
    if (ImGui::Button("Instant Repair")) m_cmd.instantRepair = true;
    ImGui::EndDisabled();

    ImGui::End();
}
