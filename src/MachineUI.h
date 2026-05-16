// MachineUI.h
#pragma once
#include "MachineController.h"

class MachineUI {
public:
    explicit MachineUI(MachineController& ctrl) : m_ctrl(ctrl) {}

    // Call this once per ImGui frame
    void render();

private:
    MachineController& m_ctrl;   // reference, does NOT own the controller
};