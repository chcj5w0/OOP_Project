#include "Machine.h"
#include <random>

int Machine::s_autoRepairDelay = 5;

namespace {
// Shared RNG for random breakdown rolls. Seeded once from the OS entropy
// source so each run differs; lives here so no machine owns global state.
std::mt19937& machineRng() {
    static std::mt19937 engine{std::random_device{}()};
    return engine;
}
}

Machine::Machine(int id, int processTicks)
    : SimObject(id), m_processTicks(processTicks) {}

void Machine::applyCmd(const MachineCmd& cmd) {
    if (cmd.targetId != id()) return;

    if (cmd.forceBreak) {
        breakDown();
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
    snap.inputId        = m_in  ? m_in->id()  : -1;
    snap.outputId       = m_out ? m_out->id() : -1;
    snap.blockedReason  = m_blockedReason;
    return snap;
}

void Machine::breakDown() {
    m_state           = MachineState::BROKEN;
    m_autoRepairTimer = s_autoRepairDelay;
    m_pendingOutput.reset();
    m_blockedReason   = "BROKEN";
    m_progress        = 0;
}

void Machine::advanceProgress() {
    if (m_state != MachineState::WORKING && m_state != MachineState::BLOCKED) return;

    if (m_state == MachineState::WORKING) {
        // Wear: health drops each working tick; reaching zero is a breakdown.
        m_health -= 0.02f;
        if (m_health <= 0.0f) {
            m_health = 0.0f;
            breakDown();
            return;
        }
        // Random failure: independent of wear, scenario-tunable.
        if (m_breakdownProb > 0.0f) {
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            if (dist(machineRng()) < m_breakdownProb) {
                breakDown();
                return;
            }
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
    } else if (m_dropOnOverflow) {
        // Output buffer full: the product spills over and is lost rather than
        // backing the line up. push() already consumed (destroyed) it.
        ++m_lostProducts;
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
