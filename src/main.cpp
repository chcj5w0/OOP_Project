#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>

#include "core/Factory.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include "ui/SimControlUI.h"
#include "ui/FactoryFloorUI.h"
#include "ui/InspectorUI.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return -1;
    }

    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Petrochemical Factory Sim - Phase 1",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          1280, 720, window_flags);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ── Backend ───────────────────────────────────────────────────────────────
    Factory factory;
    factory.buildScenarioNormal();

    // ── UI layer ──────────────────────────────────────────────────────────────
    SimControlUI   simControl;
    FactoryFloorUI floorUI;
    InspectorUI    inspectorUI;

    bool   running         = true;
    bool   paused          = false;
    int    tickIntervalMs  = 500;
    Uint32 lastTickMs      = SDL_GetTicks();
    int    inspectorTarget = 1;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) running = false;
        }

        // ── Tick the backend ──────────────────────────────────────────────────
        if (!paused) {
            Uint32 now = SDL_GetTicks();
            while (now - lastTickMs >= (Uint32)tickIntervalMs) {
                factory.tick();
                lastTickMs += tickIntervalMs;
            }
        }
        const auto snaps = factory.snapshotAll();

        // ── Build UI ──────────────────────────────────────────────────────────
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Run Control bar (Play/Pause/Step + tick speed)
        ImGui::Begin("Run Control");
        ImGui::Text("Tick: %d", factory.currentTick());
        ImGui::SameLine();
        if (ImGui::Button(paused ? "Resume" : "Pause")) paused = !paused;
        ImGui::SameLine();
        if (ImGui::Button("Step") && paused) factory.tick();
        ImGui::SliderInt("Tick interval (ms)", &tickIntervalMs, 10, 2000);
        if (ImGui::Button("Reset timer")) lastTickMs = SDL_GetTicks();
        ImGui::End();

        // Command panel — UI writes into a fresh Cmd; main forwards to backend
        MachineCmd cmd{};
        cmd.targetId = inspectorTarget;
        simControl.render(cmd);
        inspectorTarget = cmd.targetId;

        // Factory floor: all machines
        floorUI.render(snaps);

        // Inspector: snap matching selected targetId
        MachineSnap empty{};
        empty.id       = -1;
        empty.typeName = "(none)";
        empty.state    = MachineState::IDLE;
        const MachineSnap* sel = nullptr;
        for (const auto& s : snaps) {
            if (s.id == inspectorTarget) { sel = &s; break; }
        }
        inspectorUI.render(sel ? *sel : empty);

        if (cmd.startWork || cmd.forceBreak || cmd.instantRepair) {
            factory.applyCmd(cmd);
        }

        // ── Render ────────────────────────────────────────────────────────────
        ImGui::Render();
        int display_w, display_h;
        SDL_GL_GetDrawableSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
