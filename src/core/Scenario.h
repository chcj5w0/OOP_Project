#pragma once

#include <vector>

class Factory;

// A selectable simulation scenario (§2.1). Each concrete scenario knows how to
// configure a Factory for itself. The Factory and the UI never switch on the
// scenario type — they call configure()/name() polymorphically, so adding a
// new scenario means adding one subclass and registering it, with zero changes
// to the simulation loop or the UI (Open/Closed Principle).
class Scenario {
public:
    virtual ~Scenario() = default;
    virtual const char* name() const = 0;
    virtual const char* description() const = 0;
    virtual void        configure(Factory& factory) const = 0;
};

// The registry of all scenarios, in display order. The dropdown iterates this;
// adding a scenario here is the only wiring needed.
const std::vector<const Scenario*>& allScenarios();
