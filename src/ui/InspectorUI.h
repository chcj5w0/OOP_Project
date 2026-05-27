#pragma once
#include "bridge/MachineSnap.h"
#include <vector>

class InspectorUI {
public:
    void render(const std::vector<MachineSnap>& snaps);
};
