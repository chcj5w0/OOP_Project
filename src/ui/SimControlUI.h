#pragma once
#include "../bridge/MachineCmd.h"
#include "../bridge/MachineSnap.h"
#include "UIObject.h"
#include <vector>

class SimControlUI : public UIObject {
public:
    SimControlUI(MachineCmd& cmd, const std::vector<MachineSnap>& snaps)
        : m_cmd(cmd), m_snaps(snaps) {}
    void render() override;

private:
    MachineCmd&                     m_cmd;
    const std::vector<MachineSnap>& m_snaps;
};
