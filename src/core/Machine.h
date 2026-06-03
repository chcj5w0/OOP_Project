#pragma once

#include "SimObject.h"
#include "Connector.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include <memory>
#include <string>

class Machine : public SimObject {
public:
    Machine(int id, int processTicks);
    ~Machine() override = default;

    void applyCmd(const MachineCmd& cmd);
    MachineSnap snapshot() const;

    void attachInput(Connector* c)  { m_in  = c; }
    void attachOutput(Connector* c) { m_out = c; }

    Connector* input()  const { return m_in;  }
    Connector* output() const { return m_out; }

    MachineState state()        const { return m_state; }
    float        health()       const { return m_health; }
    int          progress()     const { return m_progress; }
    int          processTicks() const { return m_processTicks; }

    static int autoRepairDelay() { return s_autoRepairDelay; }
    static void setAutoRepairDelay(int delay) { if (delay < 1) delay = 1; s_autoRepairDelay = delay; }

protected:
    virtual const char* typeName() const = 0;
    virtual std::unique_ptr<Product> createOutput() = 0;
    virtual int producedCount() const { return 0; }
    virtual const char* blockedReason() const { return m_blockedReason.c_str(); }

    void advanceProgress();
    void tickAutoRepair();
    void setState(MachineState s) { m_state = s; if (s != MachineState::BLOCKED) m_blockedReason.clear(); }
    void setHealth(float h)       { m_health = h; }
    void clearBlockedReason()     { m_blockedReason.clear(); }

private:
    MachineState m_state         = MachineState::IDLE;
    float        m_health        = 1.0f;
    int          m_progress      = 0;
    int          m_processTicks  = 1;
    Connector*   m_in            = nullptr;
    Connector*   m_out           = nullptr;
    std::unique_ptr<Product> m_pendingOutput;
    std::string              m_blockedReason;
    int                      m_autoRepairTimer = 0;

    static int               s_autoRepairDelay;
};
