#include "Separator.h"

Separator::Separator(int id, int processTicks)
    : Machine(id, processTicks) {}

void Separator::update(int tick) {
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

std::unique_ptr<Product> Separator::createOutput() {
    return std::make_unique<FinishedProduct>();
}

std::string Separator::getInfo() const {
    return "Separator[" + std::to_string(id()) + "] " +
           std::to_string(progress()) + "/" + std::to_string(processTicks());
}
