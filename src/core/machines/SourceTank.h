#pragma once

#include "../Machine.h"

class SourceTank : public Machine {
public:
    SourceTank(int id, int emitInterval);
    void        update(int tick) override;
    std::string getInfo() const override;

protected:
    const char* typeName() const override { return "SourceTank"; }
    std::unique_ptr<Product> createOutput() override;

private:
    int m_emitInterval;
};
