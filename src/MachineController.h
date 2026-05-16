// MachineController.h
#pragma once
#include "Machine.h"

class MachineController {
public:
    explicit MachineController(Machine& machine) : m_machine(machine) {}

    // Called by UI when button is pressed
    void onToggleClicked() { m_machine.toggle(); }

    // Read-only query for the UI to display status
    Status      getStatus()      const { return m_machine.getStatus(); }
    const char* getStatusLabel() const { return m_machine.getStatusLabel(); }

private:
    Machine& m_machine;   // reference, does NOT own the machine
};