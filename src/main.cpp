#include <SDL.h>
#include <SDL_opengl.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <cstdio>

#include <vector>

#include "core/Factory.h"
#include "core/Scenario.h"
#include "bridge/ConnectorSnap.h"
#include "bridge/MachineCmd.h"
#include "bridge/MachineSnap.h"
#include "ui/UIObject.h"
#include "ui/SimControlUI.h"
#include "ui/FactoryFloorUI.h"
#include "ui/InspectorUI.h"
#include "ui/StatisticsUI.h"
#include "ui/RunControlManager.h"
#include "ui/EventLogUI.h"
#include "ui/UILayout.h"

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
    SDL_Window* window = SDL_CreateWindow("Petrochemical Factory Simulation",
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
    factory.loadScenario(*allScenarios().front());   // start on "Normal flow"

    // ── UI layer ──────────────────────────────────────────────────────────────
    // Shared frame data: views hold references to these and read them in render().
    // They outlive every UI object below.
    std::vector<MachineSnap>   snaps;
    std::vector<ConnectorSnap> connSnaps;
    FactoryStats               stats{};
    MachineCmd                 cmd{};
    cmd.targetId = 1;

    SimControlUI      simControl(cmd, snaps);
    FactoryFloorUI    floorUI(snaps, connSnaps, cmd);
    InspectorUI       inspectorUI(snaps);
    StatisticsUI      statisticsUI(stats);
    RunControlManager runControlManager;
    EventLogUI        eventLogUI;

    std::vector<UIObject*> uiObjects = {
        &simControl, &floorUI, &inspectorUI, &statisticsUI, &eventLogUI,
    };

    // Connect event logging via the Observer interface (no core→ui coupling).
    factory.addObserver(&eventLogUI);
    eventLogUI.addEvent(0, "Simulation started");

    bool running = true;

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
        runControlManager.update(factory);
        snaps     = factory.snapshotAll();
        connSnaps = factory.snapshotConnectors();
        stats     = factory.stats();

        // ── Build UI ──────────────────────────────────────────────────────────
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        UILayout::beginFrame();

        runControlManager.render(factory);

        for (UIObject* ui : uiObjects) {
            ui->render();
        }

        if (cmd.startWork || cmd.forceBreak || cmd.instantRepair) {
            factory.applyCmd(cmd);
        }
        // Command flags are one-shot; targetId persists across frames.
        cmd.startWork = cmd.forceBreak = cmd.instantRepair = false;

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
