#pragma once

#include "../Machine.h"

class Reactor : public Machine {
public:
    Reactor(int id, int processTicks);
    void        update(int tick) override;
    std::string getInfo() const override;

protected:
    const char* typeName() const override { return "Reactor"; }
    std::unique_ptr<Product> createOutput() override;
};
