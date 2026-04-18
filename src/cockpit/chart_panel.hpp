// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <cstdint>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <vector>

#include "currency.hpp"
#include "data_rectangle.hpp"
#include "entities/trade.hpp"
#include "pixel_rect.hpp"

namespace scratcher::cockpit {

struct IChartPanel
{
    virtual ~IChartPanel() = default;

    virtual const Rectangle& GetDataViewRect() const = 0;
    virtual Rectangle& MutableDataViewRect() = 0;
    virtual const PixelRect& GetClientRect() const = 0;
    virtual PixelRect& MutableClientRect() = 0;

    virtual int DataXToWidgetX(uint64_t x) const = 0;
    virtual int DataYToWidgetY(uint64_t y) const = 0;

    virtual currency<uint64_t> PricePoint() const = 0;
    virtual currency<uint64_t> VolumePoint() const = 0;

    virtual const std::deque<data::PublicTrade>& TradeCache() const = 0;
    virtual std::shared_lock<std::shared_mutex> LockTradeCache() const = 0;

    virtual std::vector<uint64_t> GetTimeTicksMs(uint64_t period_ms) const = 0;
    virtual std::vector<uint64_t> GetPriceTicks(uint64_t step) const = 0;
};

}
