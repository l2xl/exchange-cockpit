// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "margin.hpp"

#include <limits>

#include "chart_panel.hpp"

namespace scratcher::cockpit {

void Margin::CalculatePaint(IChartPanel& w)
{
    Rectangle& rect = w.MutableDataViewRect();
    if (rect.h == 0) return;
    if (rect.y_start() == std::numeric_limits<uint64_t>::max()) return;

    if (rect.h < 100) {
        rect.y = rect.y + rect.h / 2 - 50;
        rect.h = 100;
    } else {
        uint64_t margin = static_cast<uint64_t>(rect.h * mMarginRate);
        rect.y = rect.y > margin ? rect.y - margin : 0;
        rect.h += margin;
    }
}

}
