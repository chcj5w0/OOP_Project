#pragma once
#include "UIObject.h"
#include <string>
#include <vector>

struct Event {
    int tick;
    std::string message;
};

class EventLogUI : public UIObject {
public:
    void render();
    void addEvent(int tick, const std::string& message);
    
private:
    std::vector<Event> m_events;
    static constexpr int MAX_EVENTS = 100;
};
