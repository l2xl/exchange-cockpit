// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <include/core/SkFont.h>

#include "scratchers/time_ruler.hpp"

namespace scratcher::elements {

class TimeRuler final : public cockpit::TimeRuler
{
    SkFont mFont;

public:
    TimeRuler();

    void Paint(cockpit::IChartPanel&) const override;
};

}
