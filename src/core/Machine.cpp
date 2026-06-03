#include "Machine.h"

int Machine::s_autoRepairDelay = 5;

Machine::Machine(int id, int processTicks)
    : SimObject(id), m_processTicks(processTicks) {}

void Machine::applyCmd(const MachineCmd& cmd) {
    if (cmd.targetId != id()) return;

    if (cmd.forceBreak) {
        m_state = MachineState::BROKEN;
        m_autoRepairTimer = s_autoRepairDelay;
        m_pendingOutput.reset();
        m_blockedReason = "BROKEN";
    } else if (cmd.instantRepair) {
        m_state    = MachineState::IDLE;
        m_health   = 1.0f;
        m_progress = 0;
        m_autoRepairTimer = 0;
        m_pendingOutput.reset();
        m_blockedReason.clear();
    } else if (cmd.startWork && m_state == MachineState::IDLE) {
        m_state = MachineState::WORKING;
        m_blockedReason.clear();
    }
}

MachineSnap Machine::snapshot() const {
    MachineSnap snap{};
    snap.id             = id();
    snap.typeName       = typeName();
    snap.state          = m_state;
    snap.health         = m_health;
    snap.progress       = m_progress;
    snap.processTicks   = m_processTicks;
    snap.inputLoad      = m_in ? m_in->size() : 0;
    snap.inputCapacity  = m_in ? m_in->capacity() : 0;
    snap.outputCount    = m_out ? m_out->size() : 0;
    snap.outputCapacity = m_out ? m_out->capacity() : 0;
    snap.producedCount  = producedCount();
    snap.repairCountdown= m_state == MachineState::BROKEN ? m_autoRepairTimer : 0;
    snap.blockedReason  = m_blockedReason;
    return snap;
}

void Machine::advanceProgress() {
    if (m_state != MachineState::WORKING && m_state != MachineState::BLOCKED) return;

    if (m_state == MachineState::WORKING) {
        m_health -= 0.02f;
        if (m_health <= 0.0f) {
            m_health = 0.0f;
            m_state  = MachineState::BROKEN;
            m_autoRepairTimer = s_autoRepairDelay;
            m_pendingOutput.reset();
            m_blockedReason = "BROKEN";
            m_progress = 0;
            return;
        }
    }

    if (m_progress < m_processTicks) {
        if (m_state == MachineState::WORKING) {
            ++m_progress;
        }
        return;
    }

    if (!m_pendingOutput) {
        m_pendingOutput = createOutput();
    }

    if (!m_pendingOutput) {
        m_progress = 0;
        m_state    = MachineState::IDLE;
        m_blockedReason.clear();
        return;
    }

    if (!output()) {
        m_state = MachineState::BLOCKED;
        m_blockedReason = "NO_OUTPUT";
        return;
    }

    if (output()->push(std::move(m_pendingOutput))) {
        m_progress = 0;
        m_state    = MachineState::IDLE;
        m_blockedReason.clear();
    } else {
        m_state = MachineState::BLOCKED;
        m_blockedReason = "OUTPUT_FULL";
    }
}

void Machine::tickAutoRepair() {
    if (m_state != MachineState::BROKEN) return;
    if (m_autoRepairTimer <= 0) return;

    --m_autoRepairTimer;
    if (m_autoRepairTimer <= 0) {
        m_state  = MachineState::IDLE;
        m_health = 0.5f;
        m_blockedReason = "AUTO_REPAIRED";
    }
}
