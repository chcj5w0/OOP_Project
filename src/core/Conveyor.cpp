#include "Conveyor.h"

Conveyor::Conveyor(int id, int capacity)
    : Connector(id), m_capacity(capacity), m_slots(capacity) {}

void Conveyor::update(int tick) {
    (void)tick;
}

bool Conveyor::push(std::unique_ptr<Product> p) {
    for (auto& slot : m_slots) {
        if (!slot) {
            slot = std::move(p);
            return true;
        }
    }
    return false;
}

std::unique_ptr<Product> Conveyor::pop() {
    for (auto& slot : m_slots) {
        if (slot) {
            auto item = std::move(slot);
            slot.reset();
            return item;
        }
    }
    return nullptr;
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
