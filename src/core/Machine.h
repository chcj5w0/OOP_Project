#pragma once

#include "SimObject.h"
#include "Connector.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"

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

protected:
    virtual const char* typeName() const = 0;
    virtual void onProcessComplete() = 0;

    void advanceProgress();
    void setState(MachineState s) { m_state = s; }
    void setHealth(float h)       { m_health = h; }

private:
    MachineState m_state         = MachineState::IDLE;
    float        m_health        = 1.0f;
    int          m_progress      = 0;
    int          m_processTicks  = 1;
    Connector*   m_in            = nullptr;
    Connector*   m_out           = nullptr;
};
