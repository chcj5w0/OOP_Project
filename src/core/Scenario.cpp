#include "Scenario.h"
#include "Factory.h"

// Each scenario differs only in how it tunes the pipeline, so configure() just
// hands a ScenarioParams to Factory::build(). The virtual hook leaves the door
// open for a future scenario to build a different topology entirely.

namespace {

class NormalScenario : public Scenario {
public:
    const char* name() const override { return "Normal flow"; }
    const char* description() const override {
        return "Balanced pipeline; all machines run at default speed.";
    }
    void configure(Factory& f) const override {
        f.build({});   // all defaults
    }
};

class BottleneckScenario : public Scenario {
public:
    const char* name() const override { return "Bottleneck"; }
    const char* description() const override {
        return "Separator slows to 12 ticks, backing up its input pipe.";
    }
    void configure(Factory& f) const override {
        f.build({ /*emit*/3, /*reactor*/4, /*separator*/12, /*terminal*/1,
                  /*pipeCap*/8, /*convCap*/8, /*breakdown*/0.0f, /*drop*/false });
    }
};

class RandomBreakdownsScenario : public Scenario {
public:
    const char* name() const override { return "Random breakdowns"; }
    const char* description() const override {
        return "6% per-tick failure chance; auto-repair acts as the technician.";
    }
    void configure(Factory& f) const override {
        f.build({ /*emit*/3, /*reactor*/4, /*separator*/3, /*terminal*/1,
                  /*pipeCap*/8, /*convCap*/8, /*breakdown*/0.06f, /*drop*/false });
    }
};

class OverflowScenario : public Scenario {
public:
    const char* name() const override { return "Overflow"; }
    const char* description() const override {
        return "Slow terminal overflows the conveyor; products lost and logged.";
    }
    void configure(Factory& f) const override {
        f.build({ /*emit*/2, /*reactor*/2, /*separator*/2, /*terminal*/10,
                  /*pipeCap*/8, /*convCap*/4, /*breakdown*/0.0f, /*drop*/true });
    }
};

// Static instances live for the whole program; the registry holds pointers.
const NormalScenario           s_normal;
const BottleneckScenario       s_bottleneck;
const RandomBreakdownsScenario s_randomBreakdowns;
const OverflowScenario         s_overflow;

const std::vector<const Scenario*> s_all = {
    &s_normal, &s_bottleneck, &s_randomBreakdowns, &s_overflow,
};

} // namespace

const std::vector<const Scenario*>& allScenarios() {
    return s_all;
}
