#pragma once

class Factory;

class RunControlManager {
public:
    explicit RunControlManager(int tickIntervalMs = 500);

    void update(Factory& factory);
    void render(Factory& factory);

    void togglePause();
    void step(Factory& factory);
    void resetTimer();

    bool isPaused() const { return m_paused; }
    int tickIntervalMs() const { return m_tickIntervalMs; }
    void setTickIntervalMs(int ms);

private:
    bool   m_paused         = false;
    int    m_tickIntervalMs = 500;
    unsigned int m_lastTickMs = 0;
};
