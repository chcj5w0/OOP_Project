#pragma once

#include <string>

class SimObject {
public:
    virtual ~SimObject() = default;
    virtual void update(int tick) = 0;
    virtual std::string getInfo() const = 0;
    int id() const { return m_id; }

protected:
    explicit SimObject(int id) : m_id(id) {}

private:
    int m_id;
};
