// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <cstdint>
#include <vector>

#include "currency.hpp"
#include "scratcher.hpp"

namespace scratcher::cockpit {

class PriceRuler : public Scratcher
{
protected:
    currency<uint64_t> mPoint;
    int mTextHeight = 0;
    int mCharWidth = 0;

    std::vector<uint64_t> mTicks;
    int mAxisX = 0;
    int mTickX1 = 0;
    int mTickX2 = 0;
    int mAxisTop = 0;
    int mAxisBottom = 0;
    int mLabelX = 0;

public:
    explicit PriceRuler(currency<uint64_t> p) : mPoint(std::move(p)) {}

    void CalculateSize(IChartPanel&) override;
    void CalculatePaint(IChartPanel&) override;
};

}
