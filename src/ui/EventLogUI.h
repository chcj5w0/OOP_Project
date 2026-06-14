#pragma once
#include "UIObject.h"
#include "bridge/IEventObserver.h"
#include <string>
#include <vector>

struct Event {
    int tick;
    std::string message;
};

// A scrollable event log that observes the Factory (Observer pattern):
// onEvent() is the notification hook the Factory calls.
class EventLogUI : public UIObject, public IEventObserver {
public:
    void render() override;
    void onEvent(int tick, const std::string& message) override { addEvent(tick, message); }
    void addEvent(int tick, const std::string& message);
    void clear() { m_events.clear(); }

private:
    std::vector<Event> m_events;
    static constexpr int MAX_EVENTS = 100;
};
