#pragma once
#include "../bridge/MachineCmd.h"
#include "UIObject.h"

class SimControlUI : public UIObject {
public:
    void render() override;
    void render(MachineCmd& cmd);
};
