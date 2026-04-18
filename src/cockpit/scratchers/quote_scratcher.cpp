// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "quote_scratcher.hpp"

#include <algorithm>
#include <limits>

#include "entities/trade.hpp"
#include "timedef.hpp"

namespace scratcher::cockpit {

void QuoteScratcher::IngestNewTrades(IChartPanel& w)
{
    auto lock = w.LockTradeCache();
    const auto& cache = w.TradeCache();
    if (cache.empty()) return;

    auto start = cache.begin();
    if (mQuotes.last_trade_timestamp()) {
        uint64_t last = *mQuotes.last_trade_timestamp();
        start = std::upper_bound(cache.begin(), cache.end(), last,
            [](uint64_t v, const data::PublicTrade& t) { return v < get_timestamp(t.trade_time); });
    }
    if (start == cache.end()) return;

    uint64_t last_price = (start != cache.begin() ? (start - 1)->price_points : start->price_points);
    uint64_t now_ts = get_timestamp(std::chrono::utc_clock::now());
    mQuotes.AppendTrades(std::ranges::subrange(start, cache.end()), now_ts, last_price);
}

void QuoteScratcher::CalculatePaint(IChartPanel& w)
{
    IngestNewTrades(w);

    uint64_t maxv = 0;
    uint64_t minv = std::numeric_limits<uint64_t>::max();
    for (const auto& b : mQuotes.quotes()) {
        maxv = std::max(maxv, b.max);
        minv = std::min(minv, b.min);
    }
    auto active = mQuotes.active_candle();
    if (active.volume > 0) {
        maxv = std::max(maxv, active.max);
        minv = std::min(minv, active.min);
    }

    if (minv != std::numeric_limits<uint64_t>::max() && maxv >= minv) {
        Rectangle& rect = w.MutableDataViewRect();
        rect.y = minv;
        rect.h = std::max<uint64_t>(maxv - minv, 1);
    }
}

}
