#pragma once
#include "UIObject.h"
#include <vector>
#include "bridge/MachineSnap.h"

class FactoryFloorUI : public UIObject {
public:
    void render() override;
    void render(const std::vector<MachineSnap>& snaps);
};
