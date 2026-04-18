// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include "chart_panel.hpp"

namespace scratcher::cockpit {

struct Scratcher
{
    virtual ~Scratcher() = default;

    virtual void CalculateSize(IChartPanel&) = 0;
    virtual void CalculatePaint(IChartPanel&) = 0;
    virtual void Paint(IChartPanel&) const = 0;
};

}
