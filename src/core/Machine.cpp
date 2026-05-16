#include "Machine.h"

Machine::Machine(int id, int processTicks)
    : SimObject(id), m_processTicks(processTicks) {}

void Machine::applyCmd(const MachineCmd& cmd) {
    if (cmd.targetId != id()) return;

    if (cmd.forceBreak) {
        m_state = MachineState::BROKEN;
    } else if (cmd.instantRepair) {
        m_state    = MachineState::IDLE;
        m_health   = 1.0f;
        m_progress = 0;
    } else if (cmd.startWork && m_state == MachineState::IDLE) {
        m_state = MachineState::WORKING;
    }
}

MachineSnap Machine::snapshot() const {
    MachineSnap snap{};
    snap.id           = id();
    snap.typeName     = typeName();
    snap.state        = m_state;
    snap.health       = m_health;
    snap.progress     = m_progress;
    snap.processTicks = m_processTicks;
    snap.outputCount  = m_out ? m_out->size() : 0;
    snap.inputLoad    = m_in  ? m_in->size()  : 0;
    return snap;
}

void Machine::advanceProgress() {
    if (m_state != MachineState::WORKING) return;
    ++m_progress;
    if (m_progress >= m_processTicks) {
        onProcessComplete();
        m_progress = 0;
        m_state    = MachineState::IDLE;
    }
}
