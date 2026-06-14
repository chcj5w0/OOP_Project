#include "EventLogUI.h"
#include "UILayout.h"
#include <imgui.h>

void EventLogUI::addEvent(int tick, const std::string& message) {
    m_events.push_back({tick, message});
    if (m_events.size() > MAX_EVENTS) {
        m_events.erase(m_events.begin());
    }
}

void EventLogUI::render() {
    UILayout::placeEventLog();
    ImGui::Begin("Event Log");
    
    ImGui::Text("Total events: %zu", m_events.size());
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        clear();
    }
    ImGui::Separator();
    
    ImGui::BeginChild("EventLogScrollArea", ImVec2(0, 0), true);
    for (const auto& event : m_events) {
        ImGui::Text("[Tick %d] %s", event.tick, event.message.c_str());
    }
    ImGui::EndChild();
    
    ImGui::End();
}
