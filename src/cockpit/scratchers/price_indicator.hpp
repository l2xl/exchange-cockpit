// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include "currency.hpp"
#include "scratcher.hpp"

namespace scratcher::cockpit {

class PriceIndicator : public Scratcher
{
protected:
    currency<uint64_t> mPoint;

public:
    explicit PriceIndicator(currency<uint64_t> p) : mPoint(std::move(p)) {}

    void CalculateSize(IChartPanel&) override {}
    void CalculatePaint(IChartPanel&) override {}
};

}
