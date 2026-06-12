#pragma once

#include "UIObject.h"
#include "bridge/FactoryStats.h"

class StatisticsUI : public UIObject {
public:
    explicit StatisticsUI(const FactoryStats& stats) : m_stats(stats) {}
    void render() override;

private:
    const FactoryStats& m_stats;
};
