#include "SourceTank.h"

SourceTank::SourceTank(int id, int emitInterval)
    : Machine(id, /*processTicks*/1), m_emitInterval(emitInterval) {}

void SourceTank::update(int tick) {
    tickAutoRepair();
    if (state() == MachineState::BROKEN) return;
    if (m_emitInterval <= 0) return;

    if (state() == MachineState::BLOCKED) {
        if (output() && output()->push(std::make_unique<RawFeed>())) {
            setState(MachineState::IDLE);
        }
        return;
    }

    if (tick % m_emitInterval != 0) return;
    if (!output()) return;

    if (!output()->push(std::make_unique<RawFeed>())) {
        setState(MachineState::BLOCKED);
    }
}

std::unique_ptr<Product> SourceTank::createOutput() {
    return nullptr;
}

std::string SourceTank::getInfo() const {
    return "SourceTank[" + std::to_string(id()) + "] emit/" +
           std::to_string(m_emitInterval) + " ticks";
}
