#include "FactoryFloorUI.h"
#include "StateStyle.h"
#include "UILayout.h"
#include <imgui.h>
#include <cstdio>
#include <cstring>
#include <unordered_map>

namespace {

constexpr float kMachineW = 132.0f;
constexpr float kMachineH = 100.0f;
constexpr float kConnW    = 160.0f;
constexpr float kRowH     = 190.0f;
constexpr float kMarginX  = 14.0f;

ImU32 toU32(const ImVec4& c) { return ImGui::ColorConvertFloat4ToU32(c); }

void drawGauge(ImDrawList* dl, ImVec2 pos, float width, float height,
               float ratio, const ImVec4& fillColor) {
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    dl->AddRectFilled(pos, ImVec2(pos.x + width, pos.y + height),
                      IM_COL32(30, 32, 38, 255), 2.0f);
    if (ratio > 0.0f) {
        dl->AddRectFilled(pos, ImVec2(pos.x + width * ratio, pos.y + height),
                          toU32(fillColor), 2.0f);
    }
    dl->AddRect(pos, ImVec2(pos.x + width, pos.y + height),
                IM_COL32(80, 84, 92, 255), 2.0f);
}

// Draws the dot with the draw list because the default ImGui font has no
// "●" glyph.
void legendDot(const ImVec4& color, const char* label) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    const float  h = ImGui::GetTextLineHeight();
    dl->AddCircleFilled(ImVec2(p.x + h * 0.5f, p.y + h * 0.55f),
                        h * 0.32f, toU32(color));
    ImGui::Dummy(ImVec2(h, h));
    ImGui::SameLine(0.0f, 3.0f);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(0.0f, 14.0f);
}

} // namespace

void FactoryFloorUI::render() {
    UILayout::placeFactoryFloor();
    ImGui::Begin("Factory Floor");

    if (m_snaps.empty()) {
        ImGui::TextDisabled("No machine snapshot data available.");
        ImGui::End();
        return;
    }

    drawLegend();
    ImGui::Separator();

    ImGui::BeginChild("floor_canvas", ImVec2(0.0f, kRowH + 16.0f), false,
                      ImGuiWindowFlags_HorizontalScrollbar);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 origin  = ImGui::GetCursorScreenPos();
    const float  centerY = origin.y + kRowH * 0.5f;

    std::unordered_map<int, const ConnectorSnap*> connById;
    for (const auto& c : m_conns) connById[c.id] = &c;

    float x = origin.x + kMarginX;
    for (const auto& snap : m_snaps) {
        drawMachine(dl, snap, x, centerY);
        x += kMachineW;

        if (snap.outputId >= 0) {
            auto it = connById.find(snap.outputId);
            if (it != connById.end()) {
                drawConnector(dl, *it->second, x, centerY, kConnW);
            }
            x += kConnW;
        }
    }

    // Reserve the drawn area so the child gets a scrollbar when needed.
    ImGui::SetCursorScreenPos(origin);
    ImGui::Dummy(ImVec2(x - origin.x + kMarginX, kRowH));
    ImGui::EndChild();

    ImGui::TextDisabled("Left-click a machine to target it, right-click for commands.");
    ImGui::End();
}

void FactoryFloorUI::drawLegend() {
    legendDot(stateColor(MachineState::IDLE),    "IDLE");
    legendDot(stateColor(MachineState::WORKING), "WORKING");
    legendDot(stateColor(MachineState::BLOCKED), "BLOCKED");
    legendDot(stateColor(MachineState::BROKEN),  "BROKEN");
    ImGui::TextUnformatted("|");
    ImGui::SameLine(0.0f, 14.0f);
    legendDot(productColor("RawFeed"),         "RawFeed");
    legendDot(productColor("Intermediate"),    "Intermediate");
    legendDot(productColor("FinishedProduct"), "Finished");
    ImGui::NewLine();
}

void FactoryFloorUI::drawMachine(ImDrawList* dl, const MachineSnap& snap,
                                 float x, float centerY) {
    const float top = centerY - kMachineH * 0.5f;
    const ImVec2 p0(x, top);
    const ImVec2 p1(x + kMachineW, top + kMachineH);

    ImGui::PushID(snap.id);
    ImGui::SetCursorScreenPos(p0);
    ImGui::InvisibleButton("machine", ImVec2(kMachineW, kMachineH));
    const bool hovered = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_cmd.targetId = snap.id;
    }

    const bool selected = (m_cmd.targetId == snap.id);

    // Card body and state-colored border.
    dl->AddRectFilled(p0, p1,
                      hovered ? IM_COL32(52, 56, 66, 255)
                              : IM_COL32(40, 43, 51, 255), 6.0f);
    dl->AddRect(p0, p1, toU32(stateColor(snap)), 6.0f, 0,
                selected ? 3.5f : 2.0f);
    if (selected) {
        dl->AddRect(ImVec2(p0.x - 3, p0.y - 3), ImVec2(p1.x + 3, p1.y + 3),
                    IM_COL32(255, 255, 255, 110), 8.0f, 0, 1.5f);
    }

    // Title and state.
    char buf[64];
    snprintf(buf, sizeof(buf), "%s [%d]", snap.typeName, snap.id);
    dl->AddText(ImVec2(x + 8, top + 6), IM_COL32(235, 235, 235, 255), buf);
    dl->AddText(ImVec2(x + 8, top + 23), toU32(stateColor(snap)),
                stateLabel(snap.state));

    // Health and work-progress gauges.
    const float gaugeW = kMachineW - 16.0f;
    drawGauge(dl, ImVec2(x + 8, top + 44), gaugeW, 7.0f,
              snap.health, healthColor(snap.health));
    const float workRatio = snap.processTicks > 0
        ? float(snap.progress) / float(snap.processTicks) : 0.0f;
    drawGauge(dl, ImVec2(x + 8, top + 57), gaugeW, 7.0f,
              workRatio, ImVec4(0.30f, 0.65f, 0.95f, 1.0f));

    // Bottom status line: repair countdown, block reason, or output total.
    if (snap.state == MachineState::BROKEN && snap.repairCountdown > 0) {
        snprintf(buf, sizeof(buf), "repair in %d", snap.repairCountdown);
        dl->AddText(ImVec2(x + 8, top + 74), toU32(stateColor(MachineState::BROKEN)), buf);
    } else if (snap.state == MachineState::BLOCKED && !snap.blockedReason.empty()) {
        dl->AddText(ImVec2(x + 8, top + 74), toU32(stateColor(MachineState::BLOCKED)),
                    snap.blockedReason.c_str());
    } else if (snap.producedCount > 0) {
        snprintf(buf, sizeof(buf), "done: %d", snap.producedCount);
        dl->AddText(ImVec2(x + 8, top + 74), IM_COL32(150, 230, 160, 255), buf);
    }

    if (hovered) {
        ImGui::SetTooltip("%s [%d]\nstate: %s\nhealth: %.0f%%\nwork: %d/%d\n"
                          "in: %d/%d  out: %d/%d\nproduced: %d%s%s",
                          snap.typeName, snap.id, stateLabel(snap),
                          snap.health * 100.0f, snap.progress, snap.processTicks,
                          snap.inputLoad, snap.inputCapacity,
                          snap.outputCount, snap.outputCapacity,
                          snap.producedCount,
                          snap.blockedReason.empty() ? "" : "\nreason: ",
                          snap.blockedReason.c_str());
    }

    if (ImGui::BeginPopupContextItem("machine_ctx")) {
        ImGui::TextDisabled("%s [%d]", snap.typeName, snap.id);
        ImGui::Separator();
        if (ImGui::MenuItem("Start Work"))     { m_cmd.targetId = snap.id; m_cmd.startWork     = true; }
        if (ImGui::MenuItem("Force Break"))    { m_cmd.targetId = snap.id; m_cmd.forceBreak    = true; }
        if (ImGui::MenuItem("Instant Repair")) { m_cmd.targetId = snap.id; m_cmd.instantRepair = true; }
        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void FactoryFloorUI::drawConnector(ImDrawList* dl, const ConnectorSnap& snap,
                                   float x, float centerY, float width) {
    const float tubeL = x + 6.0f;
    const float tubeR = x + width - 18.0f;
    const float halfH = 12.0f;

    // Label above the tube.
    char buf[48];
    snprintf(buf, sizeof(buf), "%s %d/%d", snap.typeName, snap.size, snap.capacity);
    const ImVec2 labelSize = ImGui::CalcTextSize(buf);
    dl->AddText(ImVec2(x + (width - labelSize.x) * 0.5f, centerY - halfH - 20.0f),
                IM_COL32(170, 175, 185, 255), buf);

    // Flow arrowhead at the downstream (right) end.
    dl->AddTriangleFilled(ImVec2(tubeR + 2, centerY - 8),
                          ImVec2(tubeR + 2, centerY + 8),
                          ImVec2(tubeR + 14, centerY),
                          IM_COL32(150, 155, 165, 255));

    const bool isConveyor = (std::strcmp(snap.typeName, "Conveyor") == 0);
    if (isConveyor) {
        // Discrete belt slots. slots[0] is the exit end, so draw it on the
        // right where the downstream machine pops; products visibly travel
        // left → right as they advance through the belt.
        const int   cap   = snap.capacity;
        float slotW = (tubeR - tubeL - 2.0f) / float(cap);
        if (slotW > 20.0f) slotW = 20.0f;
        const float beltW  = slotW * cap;
        const float beltL  = tubeL + (tubeR - tubeL - beltW) * 0.5f;
        for (int v = 0; v < cap; ++v) {
            const int slot = cap - 1 - v;           // leftmost = entry end
            const float sx = beltL + v * slotW;
            const ImVec2 s0(sx + 1, centerY - halfH);
            const ImVec2 s1(sx + slotW - 1, centerY + halfH);
            dl->AddRectFilled(s0, s1, IM_COL32(33, 35, 42, 255), 2.0f);
            dl->AddRect(s0, s1, IM_COL32(90, 94, 104, 255), 2.0f);
            const std::string& name = snap.slots[slot];
            if (!name.empty()) {
                const ImVec2 c((s0.x + s1.x) * 0.5f, centerY);
                dl->AddCircleFilled(c, slotW * 0.32f, toU32(productColor(name)));
            }
        }
    } else {
        // Pipeline: one continuous tube; queued products stack toward the
        // exit (right) in pop order, slots[0] drawn closest to the exit.
        dl->AddRectFilled(ImVec2(tubeL, centerY - halfH),
                          ImVec2(tubeR, centerY + halfH),
                          IM_COL32(33, 35, 42, 255), halfH);
        dl->AddRect(ImVec2(tubeL, centerY - halfH),
                    ImVec2(tubeR, centerY + halfH),
                    IM_COL32(90, 94, 104, 255), halfH);
        float spacing = snap.capacity > 0
            ? (tubeR - tubeL - 18.0f) / float(snap.capacity) : 14.0f;
        if (spacing > 15.0f) spacing = 15.0f;
        for (std::size_t k = 0; k < snap.slots.size(); ++k) {
            const ImVec2 c(tubeR - 11.0f - spacing * float(k), centerY);
            if (c.x < tubeL + 9.0f) break;
            dl->AddCircleFilled(c, 5.5f, toU32(productColor(snap.slots[k])));
        }
    }
}
