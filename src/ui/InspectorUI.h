#pragma once
#include "UIObject.h"
#include "bridge/MachineSnap.h"
#include <vector>

class InspectorUI : public UIObject {
public:
    explicit InspectorUI(const std::vector<MachineSnap>& snaps) : m_snaps(snaps) {}
    void render() override;

private:
    const std::vector<MachineSnap>& m_snaps;
};
