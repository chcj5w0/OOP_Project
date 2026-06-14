#pragma once

#include <string>

// Observer interface for simulation events (Observer pattern). The Factory is
// the Subject: it notifies observers without knowing their concrete type, so
// core/ no longer depends on any UI class. A pure abstract interface carries no
// dependencies, so it lives in bridge/ alongside the other contracts.
class IEventObserver {
public:
    virtual ~IEventObserver() = default;
    virtual void onEvent(int tick, const std::string& message) = 0;
};
