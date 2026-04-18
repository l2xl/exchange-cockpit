// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <include/core/SkFont.h>

#include "scratchers/price_indicator.hpp"

namespace scratcher::elements {

class PriceIndicator final : public cockpit::PriceIndicator
{
    SkFont mFont;
    float mTextHeight = 0.0f;
    float mCharWidth = 0.0f;

public:
    explicit PriceIndicator(currency<uint64_t> p);

    void Paint(cockpit::IChartPanel&) const override;
};

}
