#pragma once

#include "Connector.h"
#include <deque>

class Pipeline : public Connector {
public:
    Pipeline(int id, int capacity);
    virtual ~Pipeline() override = default;

    void update(int tick) override;
    bool push(std::unique_ptr<Product> p) override;
    std::unique_ptr<Product> pop() override;
    int size() const override;
    int capacity() const override;

private:
    int m_capacity;
    std::deque<std::unique_ptr<Product>> m_queue;
};
