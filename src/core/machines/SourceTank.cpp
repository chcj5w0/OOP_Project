#include "SourceTank.h"

SourceTank::SourceTank(int id, int emitInterval)
    : Machine(id, /*processTicks*/1), m_emitInterval(emitInterval) {}

void SourceTank::update(int tick) {
    if (state() == MachineState::BROKEN) return;
    if (m_emitInterval <= 0) return;
    if (tick % m_emitInterval != 0) return;
    if (!output()) return;
    output()->push(std::make_unique<RawFeed>());
}

void SourceTank::onProcessComplete() {
    // SourceTank emits directly in update(); no batch step.
}

std::string SourceTank::getInfo() const {
    return "SourceTank[" + std::to_string(id()) + "] emit/" +
           std::to_string(m_emitInterval) + " ticks";
}
