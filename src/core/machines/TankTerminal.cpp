#include "TankTerminal.h"

TankTerminal::TankTerminal(int id, int processTicks)
    : Machine(id, processTicks) {}

void TankTerminal::update(int tick) {
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

std::string TankTerminal::getInfo() const {
    return "TankTerminal: " + std::to_string(progress()) + "/" +
           std::to_string(processTicks()) + ", finished=" +
           std::to_string(m_finishedProducts);
}

std::unique_ptr<Product> TankTerminal::createOutput() {
    ++m_finishedProducts;
    return nullptr;
}
