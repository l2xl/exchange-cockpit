// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)

#include "scratch_panel.hpp"
#include "quote_scratcher.hpp"

#include <algorithm>
#include <cstdlib>

#include <cairo/cairo.h>
#include <include/core/SkColor.h>

namespace scratcher::elements {

namespace el = cycfi::elements;

namespace {

constexpr SkColor kChartBg = SkColorSetARGB(0xFF, 0x12, 0x12, 0x18);

template <typename T>
T clamp_to_int(double v)
{
    if (v < static_cast<double>(std::numeric_limits<T>::min())) return std::numeric_limits<T>::min();
    if (v > static_cast<double>(std::numeric_limits<T>::max())) return std::numeric_limits<T>::max();
    return static_cast<T>(v);
}

}

ScratchPanel::ScratchPanel(std::weak_ptr<el::view> view,
                           std::weak_ptr<IDataController> controller,
                           std::string symbol,
                           currency<uint64_t> price_point,
                           currency<uint64_t> volume_point,
                           EnsurePrivate)
    : mView(std::move(view))
    , mController(std::move(controller))
    , mSymbol(std::move(symbol))
    , mPricePoint(std::move(price_point))
    , mVolumePoint(std::move(volume_point))
{
    uint64_t now_ms = get_timestamp(std::chrono::utc_clock::now());
    mDataViewRect.x = now_ms - duration_cast<milliseconds>(std::chrono::minutes{15}).count();
    mDataViewRect.w = duration_cast<milliseconds>(std::chrono::minutes{15}).count();
    mDataViewRect.y = 0;
    mDataViewRect.h = 0;
}

std::shared_ptr<ScratchPanel> ScratchPanel::Create(std::weak_ptr<el::view> view,
                                                    std::weak_ptr<IDataController> controller,
                                                    std::string symbol,
                                                    currency<uint64_t> price_point,
                                                    currency<uint64_t> volume_point)
{
    auto self = std::make_shared<ScratchPanel>(std::move(view), controller, symbol, std::move(price_point), std::move(volume_point), EnsurePrivate{});

    std::weak_ptr<ScratchPanel> weak = self;
    auto trade_handler = [weak](trade_subscription_t::data_type&& data) {
        if (auto p = weak.lock()) p->OnTradeUpdate(std::move(data));
    };
    auto ob_handler = [](ob_subscription_t::data_type&&) {};

    self->mTradeSub = datahub::make_data_subscription<std::deque<bybit::PublicTrade>>(std::move(trade_handler));
    self->mObSub = datahub::make_data_subscription<std::deque<OrderBookLevel>>(std::move(ob_handler));

    if (auto c = controller.lock()) {
        c->SubscribeInstrument(symbol, self->mObSub, self->mTradeSub);
    }

    return self;
}

std::shared_ptr<ScratchPanel> make_scratch_panel(std::weak_ptr<el::view> view,
                                                  std::weak_ptr<IDataController> controller,
                                                  std::string symbol,
                                                  currency<uint64_t> price_point,
                                                  currency<uint64_t> volume_point)
{
    return ScratchPanel::Create(std::move(view), std::move(controller), std::move(symbol), std::move(price_point), std::move(volume_point));
}

el::view_limits ScratchPanel::limits(el::basic_context const&) const
{
    return {{120.0f, 80.0f}, {el::full_extent, el::full_extent}};
}

void ScratchPanel::layout(el::context const& ctx)
{
    auto bounds = ctx.bounds;
    int new_w = clamp_to_int<int>(static_cast<double>(bounds.width()));
    int new_h = clamp_to_int<int>(static_cast<double>(bounds.height()));

    if (new_w != mBacking.Width() || new_h != mBacking.Height()) {
        mBacking.Resize(new_w, new_h);
        Relayout(ctx);
    }
}

void ScratchPanel::Relayout(const el::context&)
{
    if (mBacking.Width() <= 0 || mBacking.Height() <= 0) return;

    cockpit::PixelRect old_client = mClientRect;
    mClientRect = cockpit::PixelRect{0, 0, mBacking.Width(), mBacking.Height()};

    {
        std::shared_lock lock(mScratcherMutex);
        for (const auto& s : mScratchers) s->CalculateSize(*this);
    }

    if (old_client.width() > 0 && old_client.height() > 0) {
        uint64_t old_end = mDataViewRect.x_end();
        mDataViewRect.h = static_cast<uint64_t>(mClientRect.height() * mYScale);
        mDataViewRect.w = static_cast<uint64_t>(mClientRect.width() * mXScale);
        mDataViewRect.x = old_end - mDataViewRect.w;
    } else {
        if (mDataViewRect.w && mClientRect.width() > 0) mXScale = static_cast<double>(mDataViewRect.w) / mClientRect.width();
        if (mDataViewRect.h && mClientRect.height() > 0) mYScale = static_cast<double>(mDataViewRect.h) / mClientRect.height();
    }

    {
        std::shared_lock lock(mScratcherMutex);
        for (const auto& s : mScratchers) s->CalculatePaint(*this);
    }

    for (const auto& h : mDataViewListeners) h(*this);
}

void ScratchPanel::draw(el::context const& ctx)
{
    if (mBacking.Width() <= 0 || mBacking.Height() <= 0) return;

    SkCanvas* c = mBacking.Surface().getCanvas();
    c->save();
    c->clear(kChartBg);
    {
        std::shared_lock lock(mScratcherMutex);
        for (const auto& s : mScratchers) s->Paint(*this);
    }
    c->restore();

    mLastFlushAt = std::chrono::steady_clock::now();
    mRefreshPending.clear();

    if (mBacking.CairoSurface()) {
        cairo_t* cr = &ctx.canvas.cairo_context();
        cairo_save(cr);
        cairo_set_source_surface(cr, mBacking.CairoSurface(), ctx.bounds.left, ctx.bounds.top);
        cairo_rectangle(cr, ctx.bounds.left, ctx.bounds.top, ctx.bounds.width(), ctx.bounds.height());
        cairo_fill(cr);
        cairo_restore(cr);
    }
}

el::element* ScratchPanel::hit_test(el::context const& ctx, el::point p, bool, bool)
{
    if (ctx.bounds.includes(p)) return this;
    return nullptr;
}

void ScratchPanel::AddScratcher(std::shared_ptr<cockpit::Scratcher> s, std::optional<size_t> z_order)
{
    std::unique_lock lock(mScratcherMutex);
    if (z_order && *z_order <= mScratchers.size())
        mScratchers.emplace(mScratchers.begin() + *z_order, std::move(s));
    else
        mScratchers.emplace_back(std::move(s));
}

void ScratchPanel::RemoveScratcher(const std::shared_ptr<cockpit::Scratcher>& scratcher)
{
    std::unique_lock lock(mScratcherMutex);
    auto it = std::find(mScratchers.begin(), mScratchers.end(), scratcher);
    if (it != mScratchers.end()) mScratchers.erase(it);
    if (std::static_pointer_cast<cockpit::Scratcher>(mQuote) == scratcher) mQuote.reset();
}

void ScratchPanel::AddQuoteScratcher(std::shared_ptr<QuoteScratcher> q)
{
    {
        std::unique_lock lock(mScratcherMutex);
        mQuote = q;
    }
    AddScratcher(std::static_pointer_cast<cockpit::Scratcher>(std::move(q)), 0);
}

void ScratchPanel::SetDataViewRect(Rectangle rect)
{
    mDataViewRect = rect;
    if (mClientRect.width() > 0) mXScale = static_cast<double>(mDataViewRect.w) / mClientRect.width();
    if (mClientRect.height() > 0) mYScale = static_cast<double>(mDataViewRect.h) / mClientRect.height();
    Update();
}

int ScratchPanel::DataXToWidgetX(uint64_t x) const
{
    if (mDataViewRect.w == 0 || mClientRect.width() == 0) return 0;
    return static_cast<int>((x - mDataViewRect.x_start()) * (static_cast<double>(mClientRect.width()) / mDataViewRect.w));
}

int ScratchPanel::DataYToWidgetY(uint64_t y) const
{
    if (mDataViewRect.h == 0 || mClientRect.height() == 0) return 0;
    return mClientRect.height() - static_cast<int>((y - mDataViewRect.y_start()) * (static_cast<double>(mClientRect.height()) / mDataViewRect.h));
}

std::shared_ptr<QuoteScratcher> ScratchPanel::Quote() const
{
    std::shared_lock lock(mScratcherMutex);
    return mQuote;
}

std::vector<uint64_t> ScratchPanel::GetTimeTicksMs(uint64_t period_ms) const
{
    std::vector<uint64_t> ticks;
    if (period_ms == 0) return ticks;
    uint64_t x_start = mDataViewRect.x_start();
    uint64_t first = ((x_start + period_ms - 1) / period_ms) * period_ms;
    for (uint64_t t = first; t < mDataViewRect.x_end(); t += period_ms) {
        ticks.push_back(t);
    }
    return ticks;
}

std::vector<uint64_t> ScratchPanel::GetPriceTicks(uint64_t step) const
{
    std::vector<uint64_t> ticks;
    if (mDataViewRect.y_start() != std::numeric_limits<uint64_t>::max() && step) {
        for (uint64_t tick = ((mDataViewRect.y / step) + 1) * step;
             tick < mDataViewRect.y_end();
             tick += step) {
            ticks.push_back(tick);
        }
    }
    return ticks;
}

data::PublicTrade ScratchPanel::ConvertTrade(const bybit::PublicTrade& src) const
{
    time_point trade_time{};
    uint64_t price_points = 0;
    uint64_t volume_points = 0;

    if (!src.time.empty()) {
        try {
            uint64_t ms = std::stoull(src.time);
            trade_time = time_point{duration_cast<std::chrono::utc_clock::duration>(milliseconds{ms})};
        } catch (...) {}
    }
    if (!src.price.empty()) {
        try {
            currency<uint64_t> p = mPricePoint;
            p.parse(src.price);
            price_points = p.raw();
        } catch (...) {}
    }
    if (!src.size.empty()) {
        try {
            currency<uint64_t> v = mVolumePoint;
            v.parse(src.size);
            volume_points = v.raw();
        } catch (...) {}
    }

    return data::PublicTrade{src.execId, trade_time, price_points, volume_points, data::OrderSide::BUY};
}

void ScratchPanel::OnTradeUpdate(std::pair<datahub::update_kind, trade_subscription_t::range_view_type>&& update)
{
    auto& [kind, range] = update;
    std::unique_lock lock(mDataMutex);
    if (kind == datahub::update_kind::snapshot) mTradeCache.clear();
    for (const auto& trade : range) {
        mTradeCache.emplace_back(ConvertTrade(trade));
    }
}

void ScratchPanel::Update()
{
    uint64_t now_ms = get_timestamp(std::chrono::utc_clock::now());
    if (now_ms > mDataViewRect.x_end()) {
        mDataViewRect.x = now_ms - mDataViewRect.w;
    }

    {
        std::shared_lock lock(mScratcherMutex);
        for (const auto& s : mScratchers) s->CalculatePaint(*this);
    }

    if (mClientRect.width() > 0) mXScale = static_cast<double>(mDataViewRect.w) / mClientRect.width();
    if (mClientRect.height() > 0) mYScale = static_cast<double>(mDataViewRect.h) / mClientRect.height();

    for (const auto& h : mDataViewListeners) h(*this);

    RequestRefresh();
}

void ScratchPanel::RequestRefresh()
{
    if (mRefreshPending.test_and_set()) return;

    auto v = mView.lock();
    if (!v) {
        mRefreshPending.clear();
        return;
    }

    auto self = std::static_pointer_cast<ScratchPanel>(shared_from_this());
    std::weak_ptr<ScratchPanel> weak = self;

    constexpr auto frame_budget = std::chrono::milliseconds{16};
    auto now = std::chrono::steady_clock::now();
    auto since_last = now - mLastFlushAt;

    if (since_last >= frame_budget) {
        v->post([weak] { if (auto p = weak.lock()) p->KickRefresh(); });
    } else {
        mDelayedRefresh = v->post(frame_budget - since_last, [weak] { if (auto p = weak.lock()) p->KickRefresh(); });
    }
}

void ScratchPanel::KickRefresh()
{
    auto v = mView.lock();
    if (!v) {
        mRefreshPending.clear();
        return;
    }
    v->refresh(*this);
}

}
