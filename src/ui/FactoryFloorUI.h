#pragma once
#include <vector>
#include "bridge/MachineSnap.h"

class FactoryFloorUI {
public:
    void render(const std::vector<MachineSnap>& snaps);
};
