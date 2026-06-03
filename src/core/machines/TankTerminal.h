#pragma once
#include "../Machine.h"

class TankTerminal : public Machine {
public:
    TankTerminal(int id, int processTicks);
    void update(int tick) override;
    std::string getInfo() const override;
    int finishedProducts() const { return m_finishedProducts; }

protected:
    const char* typeName() const override { return "TankTerminal"; }
    std::unique_ptr<Product> createOutput() override;
    int producedCount() const override { return m_finishedProducts; }

private:
    int m_finishedProducts = 0;
};
