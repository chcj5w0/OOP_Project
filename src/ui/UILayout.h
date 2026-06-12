#pragma once
#include <imgui.h>

// Default placement of every window, in one place so the dashboard layout
// is defined once. Views call place() at the top of their render().
// Normally the position only applies when imgui.ini has no entry yet
// (ImGuiCond_FirstUseEver); after requestReset() it is forced for one full
// frame so "Reset Layout" snaps every window back to the dashboard.
//
// The request becomes active on the *next* frame (beginFrame() promotes it)
// because the button is pressed mid-frame, after some windows have already
// been drawn — applying immediately would skip those windows.
namespace UILayout {

inline bool g_pending = false;
inline bool g_active  = false;

inline void requestReset() { g_pending = true; }

// Call once per frame, right after ImGui::NewFrame().
inline void beginFrame() {
    g_active  = g_pending;
    g_pending = false;
}

inline void place(float x, float y, float w, float h) {
    const ImGuiCond cond = g_active ? ImGuiCond_Always
                                    : ImGuiCond_FirstUseEver;
    ImGui::SetNextWindowPos(ImVec2(x, y), cond);
    ImGui::SetNextWindowSize(ImVec2(w, h), cond);
}

// Dashboard layout (1280x720 reference).
inline void placeFactoryFloor() { place(20,  20,  1180, 320); }
inline void placeRunControl()   { place(20,  350, 380,  170); }
inline void placeSimControl()   { place(410, 350, 340,  170); }
inline void placeStatistics()   { place(760, 350, 220,  170); }
inline void placeEventLog()     { place(990, 350, 270,  350); }
inline void placeInspector()    { place(20,  530, 960,  170); }

} // namespace UILayout
