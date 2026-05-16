#include "EventLogUI.h"
#include <imgui.h>

void EventLogUI::render() {
    ImGui::Begin("Event Log");
    ImGui::Text("Event log area (Phase 2)");
    ImGui::End();
}
