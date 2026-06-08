#include "RunControlManager.h"
#include "core/Machine.h"
#include <SDL.h>
#include <imgui.h>
#include "core/Factory.h"

RunControlManager::RunControlManager(int tickIntervalMs)
    : m_paused(false), m_tickIntervalMs(tickIntervalMs), m_lastTickMs(SDL_GetTicks()) {}

void RunControlManager::update(Factory& factory) {
    if (m_paused) return;

    unsigned int now = SDL_GetTicks();
    while (now - m_lastTickMs >= static_cast<unsigned int>(m_tickIntervalMs)) {
        factory.tick();
        m_lastTickMs += static_cast<unsigned int>(m_tickIntervalMs);
    }
}

void RunControlManager::render(Factory& factory) {
    ImGui::Begin("Run Control");

    ImGui::Text("Tick: %d", factory.currentTick());
    ImGui::SameLine();
    if (ImGui::Button(m_paused ? "Resume" : "Pause")) {
        togglePause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step") && m_paused) {
        step(factory);
    }

    ImGui::SliderInt("Tick interval (ms)", &m_tickIntervalMs, 10, 2000);
    if (m_tickIntervalMs < 10) {
        m_tickIntervalMs = 10;
    }

    int repairDelay = Machine::autoRepairDelay();
    if (ImGui::SliderInt("Repair delay (ticks)", &repairDelay, 1, 20)) {
        Machine::setAutoRepairDelay(repairDelay);
    }

    if (ImGui::Button("Reset timer")) {
        resetTimer();
    }

    ImGui::End();
}

void RunControlManager::togglePause() {
    m_paused = !m_paused;
    if (!m_paused) {
        m_lastTickMs = SDL_GetTicks();
    }
}

void RunControlManager::step(Factory& factory) {
    factory.tick();
    m_lastTickMs = SDL_GetTicks();
}

void RunControlManager::resetTimer() {
    m_lastTickMs = SDL_GetTicks();
}

void RunControlManager::setTickIntervalMs(int ms) {
    if (ms < 10) ms = 10;
    m_tickIntervalMs = ms;
}
