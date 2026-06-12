#include "Conveyor.h"

Conveyor::Conveyor(int id, int capacity)
    : Connector(id), m_capacity(capacity), m_slots(capacity) {}

// Belt layout: slot 0 is the exit end, slot capacity-1 is the entry end.
// Each tick every product advances one slot toward the exit if the slot
// ahead is free; pop() only delivers the product sitting at the exit.
void Conveyor::update(int tick) {
    (void)tick;
    for (std::size_t i = 0; i + 1 < m_slots.size(); ++i) {
        if (!m_slots[i] && m_slots[i + 1]) {
            m_slots[i] = std::move(m_slots[i + 1]);
        }
    }
}

bool Conveyor::push(std::unique_ptr<Product> p) {
    if (m_slots.empty() || m_slots.back())
        return false;
    m_slots.back() = std::move(p);
    return true;
}

std::unique_ptr<Product> Conveyor::pop() {
    if (m_slots.empty() || !m_slots.front())
        return nullptr;
    return std::move(m_slots.front());
}

int Conveyor::size() const {
    int count = 0;
    for (const auto& slot : m_slots) {
        if (slot) ++count;
    }
    return count;
}

int Conveyor::capacity() const {
    return m_capacity;
}
