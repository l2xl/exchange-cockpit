// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <cstdint>
#include <vector>

#include "scratcher.hpp"

namespace scratcher::cockpit {

class TimeRuler : public Scratcher
{
protected:
    int mTextHeight = 0;

    std::vector<uint64_t> mTicks;
    int mAxisY = 0;
    int mTickY1 = 0;
    int mTickY2 = 0;
    int mAxisLeft = 0;
    int mAxisRight = 0;

public:
    void CalculateSize(IChartPanel&) override;
    void CalculatePaint(IChartPanel&) override;
};

}
