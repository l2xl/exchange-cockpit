// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "time_ruler.hpp"

#include <algorithm>
#include <chrono>

#include "chart_panel.hpp"

namespace scratcher::cockpit {

namespace {

uint64_t PickTickPeriod(uint64_t window_ms)
{
    using namespace std::chrono;
    uint64_t day  = duration_cast<milliseconds>(days{1}).count();
    uint64_t hour = duration_cast<milliseconds>(hours{1}).count();
    uint64_t min  = duration_cast<milliseconds>(minutes{1}).count();
    uint64_t sec  = duration_cast<milliseconds>(seconds{1}).count();

    if (window_ms / day  >= 3) return day;
    if (window_ms / hour >= 3) return hour;
    if (window_ms / min  >= 3) return min;
    if (window_ms / sec  >= 3) return sec;
    return 0;
}

}

void TimeRuler::CalculateSize(IChartPanel& w)
{
    PixelRect& r = w.MutableClientRect();
    int reserve = mTextHeight * 2 + mTextHeight / 2 + 1;
    r.bottom = std::max(r.top, r.bottom - reserve);
}

void TimeRuler::CalculatePaint(IChartPanel& w)
{
    const PixelRect& rect = w.GetClientRect();
    if (rect.empty()) {
        mTicks.clear();
        return;
    }

    mAxisY = rect.bottom + 1;
    mTickY2 = mAxisY + mTextHeight / 4;
    mTickY1 = mTickY2 - mTextHeight / 2;
    mAxisLeft = rect.left;
    mAxisRight = rect.right;

    uint64_t period = PickTickPeriod(w.GetDataViewRect().w);
    mTicks = period ? w.GetTimeTicksMs(period) : std::vector<uint64_t>{};
}

}
