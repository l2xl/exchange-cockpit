// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include "scratcher.hpp"

namespace scratcher::cockpit {

class Margin final : public Scratcher
{
    double mMarginRate;

public:
    explicit Margin(double margin_rate) : mMarginRate(margin_rate) {}

    void CalculateSize(IChartPanel&) override {}
    void CalculatePaint(IChartPanel&) override;
    void Paint(IChartPanel&) const override {}
};

}
