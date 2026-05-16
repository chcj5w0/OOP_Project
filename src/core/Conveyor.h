#pragma once

#include "Connector.h"
#include <vector>

class Conveyor : public Connector {
public:
    Conveyor(int id, int capacity);
    virtual ~Conveyor() override = default;

    void update(int tick) override;
    bool push(std::unique_ptr<Product> p) override;
    std::unique_ptr<Product> pop() override;
    int size() const override;
    int capacity() const override;

private:
    int m_capacity;
    std::vector<std::unique_ptr<Product>> m_slots;
};
