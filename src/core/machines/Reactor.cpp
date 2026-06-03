#include "Reactor.h"

Reactor::Reactor(int id, int processTicks)
    : Machine(id, processTicks) {}

void Reactor::update(int tick) {
    (void)tick;
    tickAutoRepair();
    if (state() == MachineState::BROKEN) return;

    if (state() == MachineState::IDLE && input()) {
        auto p = input()->pop();
        if (p) {
            setState(MachineState::WORKING);
        }
    }

    advanceProgress();
}

std::unique_ptr<Product> Reactor::createOutput() {
    return std::make_unique<Intermediate>();
}

std::string Reactor::getInfo() const {
    return "Reactor[" + std::to_string(id()) + "] " +
           std::to_string(progress()) + "/" + std::to_string(processTicks());
}
