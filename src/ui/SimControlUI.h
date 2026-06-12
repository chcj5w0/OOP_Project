#pragma once
#include "../bridge/MachineCmd.h"
#include "UIObject.h"

class SimControlUI : public UIObject {
public:
    explicit SimControlUI(MachineCmd& cmd) : m_cmd(cmd) {}
    void render() override;

private:
    MachineCmd& m_cmd;
};
