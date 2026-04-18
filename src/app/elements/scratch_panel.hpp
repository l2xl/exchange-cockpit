// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#pragma once

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

#include <elements.hpp>

#include <include/core/SkCanvas.h>

#include "cairo_skia_bridge.hpp"
#include "chart_panel.hpp"
#include "currency.hpp"
#include "data_controller.hpp"
#include "data_rectangle.hpp"
#include "datahub/data_subscription.hpp"
#include "datahub/data_update.hpp"
#include "entities/trade.hpp"
#include "orderbook.hpp"
#include "bybit/entities/public_trade.hpp"
#include "pixel_rect.hpp"
#include "scratcher.hpp"
#include "timedef.hpp"

namespace scratcher::elements {

class QuoteScratcher;

class ScratchPanel : public cycfi::elements::element, public cockpit::IChartPanel
{
    struct EnsurePrivate {};

    using trade_subscription_t = datahub::data_subscription<std::deque<bybit::PublicTrade>>;
    using ob_subscription_t = datahub::data_subscription<std::deque<OrderBookLevel>>;

    std::weak_ptr<cycfi::elements::view> mView;
    std::weak_ptr<IDataController> mController;
    std::string mSymbol;
    currency<uint64_t> mPricePoint;
    currency<uint64_t> mVolumePoint;

    Rectangle mDataViewRect;
    cockpit::PixelRect mClientRect;
    double mXScale = 1.0;
    double mYScale = 1.0;

    CairoSkiaBridge mBacking;

    std::deque<std::shared_ptr<cockpit::Scratcher>> mScratchers;
    std::shared_ptr<QuoteScratcher> mQuote;
    mutable std::shared_mutex mScratcherMutex;

    std::deque<data::PublicTrade> mTradeCache;
    mutable std::shared_mutex mDataMutex;

    std::atomic_flag mRefreshPending = ATOMIC_FLAG_INIT;
    std::chrono::steady_clock::time_point mLastFlushAt = std::chrono::steady_clock::time_point::min();
    cycfi::elements::view::steady_timer_ptr mDelayedRefresh;

    std::shared_ptr<trade_subscription_t> mTradeSub;
    std::shared_ptr<ob_subscription_t> mObSub;

    std::list<std::function<void(ScratchPanel&)>> mDataViewListeners;

public:
    ScratchPanel(std::weak_ptr<cycfi::elements::view> view,
                 std::weak_ptr<IDataController> controller,
                 std::string symbol,
                 currency<uint64_t> price_point,
                 currency<uint64_t> volume_point,
                 EnsurePrivate);
    ~ScratchPanel() override = default;

    static std::shared_ptr<ScratchPanel> Create(std::weak_ptr<cycfi::elements::view> view,
                                                 std::weak_ptr<IDataController> controller,
                                                 std::string symbol,
                                                 currency<uint64_t> price_point,
                                                 currency<uint64_t> volume_point);

    cycfi::elements::view_limits limits(cycfi::elements::basic_context const&) const override;
    void layout(cycfi::elements::context const&) override;
    void draw(cycfi::elements::context const&) override;
    cycfi::elements::element* hit_test(cycfi::elements::context const&, cycfi::elements::point, bool, bool) override;

    void AddScratcher(std::shared_ptr<cockpit::Scratcher> scratcher, std::optional<size_t> z_order = {});
    void RemoveScratcher(const std::shared_ptr<cockpit::Scratcher>& scratcher);
    void AddQuoteScratcher(std::shared_ptr<QuoteScratcher> scratcher);

    void SetDataViewRect(Rectangle rect);
    void AddDataViewChangeListener(std::function<void(ScratchPanel&)> handler)
    { mDataViewListeners.emplace_back(std::move(handler)); }

    void Update();

    SkCanvas& BackingCanvas() const { return *mBacking.Surface().getCanvas(); }
    std::shared_ptr<QuoteScratcher> Quote() const;

    const Rectangle& GetDataViewRect() const override { return mDataViewRect; }
    Rectangle& MutableDataViewRect() override { return mDataViewRect; }
    const cockpit::PixelRect& GetClientRect() const override { return mClientRect; }
    cockpit::PixelRect& MutableClientRect() override { return mClientRect; }

    int DataXToWidgetX(uint64_t x) const override;
    int DataYToWidgetY(uint64_t y) const override;

    currency<uint64_t> PricePoint() const override { return mPricePoint; }
    currency<uint64_t> VolumePoint() const override { return mVolumePoint; }

    const std::deque<data::PublicTrade>& TradeCache() const override { return mTradeCache; }
    std::shared_lock<std::shared_mutex> LockTradeCache() const override { return std::shared_lock<std::shared_mutex>(mDataMutex); }

    std::vector<uint64_t> GetTimeTicksMs(uint64_t period_ms) const override;
    std::vector<uint64_t> GetPriceTicks(uint64_t step) const override;

private:
    void Relayout(const cycfi::elements::context& ctx);
    void OnTradeUpdate(std::pair<datahub::update_kind, trade_subscription_t::range_view_type>&& update);
    void RequestRefresh();
    void KickRefresh();
    data::PublicTrade ConvertTrade(const bybit::PublicTrade& src) const;
};

std::shared_ptr<ScratchPanel> make_scratch_panel(std::weak_ptr<cycfi::elements::view> view,
                                                  std::weak_ptr<IDataController> controller,
                                                  std::string symbol,
                                                  currency<uint64_t> price_point,
                                                  currency<uint64_t> volume_point);

}
