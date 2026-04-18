// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <include/core/SkFont.h>

#include "scratchers/price_ruler.hpp"

namespace scratcher::elements {

class PriceRuler final : public cockpit::PriceRuler
{
    SkFont mFont;

public:
    explicit PriceRuler(currency<uint64_t> p);

    void Paint(cockpit::IChartPanel&) const override;
};

}
