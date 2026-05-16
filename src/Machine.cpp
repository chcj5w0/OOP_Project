// Machine.cpp
#include "Machine.h"

void Machine::toggle() {
    m_status = (m_status == Status::STOPPED)
             ? Status::ACTIVE
             : Status::STOPPED;
}

const char* Machine::getStatusLabel() const {
    return (m_status == Status::ACTIVE) ? "ACTIVE" : "STOPPED";
}