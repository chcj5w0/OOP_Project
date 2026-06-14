#include "RunControlManager.h"
#include "UILayout.h"
#include "core/Machine.h"
#include "core/Factory.h"
#include "core/Scenario.h"
#include <SDL.h>
#include <imgui.h>

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
    UILayout::placeRunControl();
    ImGui::Begin("Run Control");

    // Scenario selector (§2.1): iterate the registry — no switch on type here,
    // so adding a scenario needs no change to this loop.
    const Scenario* current = factory.currentScenario();
    const char* currentName = current ? current->name() : "(none)";
    if (ImGui::BeginCombo("Scenario", currentName)) {
        for (const Scenario* scn : allScenarios()) {
            if (ImGui::Selectable(scn->name(), scn == current)) {
                factory.loadScenario(*scn);
                resetTimer();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", scn->description());
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Tick: %d", factory.currentTick());
    ImGui::SameLine();
    if (ImGui::Button(m_paused ? "Resume" : "Pause")) {
        togglePause();
    }
    ImGui::SameLine();
    if (ImGui::Button("Step") && m_paused) {
        step(factory);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        factory.reset();
        resetTimer();
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
    ImGui::SameLine();
    if (ImGui::Button("Reset Layout")) {
        UILayout::requestReset();
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
