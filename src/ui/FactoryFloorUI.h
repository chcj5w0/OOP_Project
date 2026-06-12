#pragma once
#include "UIObject.h"
#include <vector>
#include "bridge/MachineSnap.h"
#include "bridge/ConnectorSnap.h"
#include "bridge/MachineCmd.h"

struct ImDrawList;
struct ImVec2;

// Graphical process-flow view. Draws every machine as a status card and
// every connector as a pipe/belt with the products currently inside it.
// Clicking a machine selects it as the command target; right-clicking
// opens a quick command menu.
class FactoryFloorUI : public UIObject {
public:
    FactoryFloorUI(const std::vector<MachineSnap>& snaps,
                   const std::vector<ConnectorSnap>& conns,
                   MachineCmd& cmd)
        : m_snaps(snaps), m_conns(conns), m_cmd(cmd) {}

    void render() override;

private:
    void drawMachine(ImDrawList* dl, const MachineSnap& snap,
                     float x, float centerY);
    void drawConnector(ImDrawList* dl, const ConnectorSnap& snap,
                       float x, float centerY, float width);
    void drawLegend();

    const std::vector<MachineSnap>&   m_snaps;
    const std::vector<ConnectorSnap>& m_conns;
    MachineCmd&                       m_cmd;
};
