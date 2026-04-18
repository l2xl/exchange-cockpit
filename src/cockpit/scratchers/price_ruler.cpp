// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "price_ruler.hpp"

#include <algorithm>
#include <string>

#include "chart_panel.hpp"

namespace scratcher::cockpit {

namespace {

uint64_t ChooseScale(uint64_t span)
{
    uint64_t scale = 1;
    uint64_t steps = span;
    while (steps > 5) { steps /= 5; scale *= 5; }
    return scale;
}

}

void PriceRuler::CalculateSize(IChartPanel& w)
{
    PixelRect& r = w.MutableClientRect();
    currency<uint64_t> max_price = mPoint;
    max_price.set_raw(w.GetDataViewRect().y_end());
    std::string label = max_price.to_string();
    int label_chars = static_cast<int>(label.size());
    r.right = std::max(r.left, r.right - label_chars * mCharWidth - mCharWidth * 2 - 1);
}

void PriceRuler::CalculatePaint(IChartPanel& w)
{
    const PixelRect& rect = w.GetClientRect();
    if (rect.empty()) {
        mTicks.clear();
        return;
    }

    mAxisX = rect.right + 1;
    mTickX1 = mAxisX - mCharWidth / 2;
    mTickX2 = mAxisX + mCharWidth / 2;
    mAxisTop = rect.top;
    mAxisBottom = rect.bottom;
    mLabelX = rect.right + 1 + mCharWidth;

    mTicks = w.GetPriceTicks(ChooseScale(w.GetDataViewRect().h));
}

}
