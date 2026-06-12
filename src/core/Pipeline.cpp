#include "Pipeline.h"

Pipeline::Pipeline(int id, int capacity)
    : Connector(id), m_capacity(capacity) {}

void Pipeline::update(int tick) {
    (void)tick;
}

bool Pipeline::push(std::unique_ptr<Product> p) {
    if (size() >= capacity())
        return false;
    m_queue.push_back(std::move(p));
    return true;
}

std::unique_ptr<Product> Pipeline::pop() {
    if (m_queue.empty())
        return nullptr;
    auto item = std::move(m_queue.front());
    m_queue.pop_front();
    return item;
}

int Pipeline::size() const {
    return static_cast<int>(m_queue.size());
}

int Pipeline::capacity() const {
    return m_capacity;
}

std::vector<std::string> Pipeline::slotNames() const {
    std::vector<std::string> names;
    names.reserve(m_queue.size());
    for (const auto& p : m_queue) {
        names.push_back(p->name());
    }
    return names;
}
