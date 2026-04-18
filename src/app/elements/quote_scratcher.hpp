// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include "scratchers/quote_scratcher.hpp"

namespace scratcher::elements {

class QuoteScratcher final : public cockpit::QuoteScratcher
{
public:
    using cockpit::QuoteScratcher::QuoteScratcher;

    void Paint(cockpit::IChartPanel&) const override;
};

}
