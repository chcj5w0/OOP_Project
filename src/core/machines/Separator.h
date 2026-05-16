#pragma once

#include "../Machine.h"

class Separator : public Machine {
public:
    Separator(int id, int processTicks);
    void        update(int tick) override;
    std::string getInfo() const override;

protected:
    const char* typeName() const override { return "Separator"; }
    void        onProcessComplete() override;
};
