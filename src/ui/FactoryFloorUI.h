#pragma once
#include "UIObject.h"
#include <vector>
#include "bridge/MachineSnap.h"

class FactoryFloorUI : public UIObject {
public:
    explicit FactoryFloorUI(const std::vector<MachineSnap>& snaps) : m_snaps(snaps) {}
    void render() override;

private:
    const std::vector<MachineSnap>& m_snaps;
};
