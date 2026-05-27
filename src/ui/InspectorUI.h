#pragma once
#include "bridge/MachineSnap.h"
class InspectorUI : public UIObject {
public:
    void render() override;
    void render(const MachineSnap& snap);
};
