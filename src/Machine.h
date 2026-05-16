// Machine.h
#pragma once

enum class Status { STOPPED, ACTIVE };

class Machine {
public:
    Machine() : m_status(Status::STOPPED) {}

    void        toggle();
    Status      getStatus() const { return m_status; }
    const char* getStatusLabel() const;

private:
    Status m_status;
};