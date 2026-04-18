// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)
// -----BEGIN PGP PUBLIC KEY BLOCK-----
//
// mDMEYdxcVRYJKwYBBAHaRw8BAQdAfacBVThCP5QDPEgSbSIudtpJS4Y4Imm5dzaN
// lM1HTem0IkwyIFhsIChsMnhsKSA8bDJ4bEBwcm90b25tYWlsLmNvbT6IkAQTFggA
// OBYhBKRCfUyWnduCkisNl+WRcOaCK79JBQJh3FxVAhsDBQsJCAcCBhUKCQgLAgQW
// AgMBAh4BAheAAAoJEOWRcOaCK79JDl8A/0/AjYVbAURZJXP3tHRgZyYyN9txT6mW
// 0bYCcOf0rZ4NAQDoFX4dytPDvcjV7ovSQJ6dzvIoaRbKWGbHRCufrm5QBA==
// =KKu7
// -----END PGP PUBLIC KEY BLOCK-----

#include "elements_instrument_panel.hpp"

#include "content_panel.hpp"
#include "currency.hpp"
#include "scratch_panel.hpp"
#include "time_ruler.hpp"
#include "price_ruler.hpp"
#include "scratchers/margin.hpp"
#include "quote_scratcher.hpp"
#include "price_indicator.hpp"

namespace scratcher::elements {

namespace el = cycfi::elements;

namespace {

std::optional<bybit::InstrumentInfo> FindInstrument(const std::weak_ptr<IDataController>& weak, const std::string& symbol)
{
    auto controller = weak.lock();
    if (!controller) return {};
    const auto& cache = controller->getInstrumentsFeed().get_snapshot();
    for (const auto& info : cache) {
        if (info.symbol == symbol) return info;
    }
    return {};
}

std::shared_ptr<ScratchPanel> PopulateMarketGraph(std::weak_ptr<el::view> view,
                         std::weak_ptr<IDataController> controller,
                         const std::string& symbol,
                         std::shared_ptr<el::deck_composite> work_area)
{
    auto info_opt = FindInstrument(controller, symbol);
    if (!info_opt) return {};

    currency<uint64_t> price_point(info_opt->tickSize);
    currency<uint64_t> volume_point(info_opt->basePrecision);

    auto panel = ScratchPanel::Create(view, controller, symbol, price_point, volume_point);
    panel->AddScratcher(std::make_shared<TimeRuler>());
    panel->AddScratcher(std::make_shared<PriceRuler>(price_point));
    panel->AddScratcher(std::make_shared<cockpit::Margin>(0.05));
    panel->AddQuoteScratcher(std::make_shared<QuoteScratcher>(std::chrono::seconds{60}));
    panel->AddScratcher(std::make_shared<PriceIndicator>(price_point));

    work_area->clear();
    work_area->push_back(el::share(el::hold(panel)));
    work_area->select(0);
    return panel;
}

}

ElementsInstrumentPanel::ElementsInstrumentPanel(cockpit::PanelType type,
    std::weak_ptr<el::view> view,
    std::weak_ptr<IDataController> controller,
    InstrumentPanelWidgets widgets,
    EnsurePrivate)
    : InstrumentPanel(type, std::move(controller))
    , mView(std::move(view))
    , mWidgets(std::move(widgets))
{}

std::shared_ptr<ElementsInstrumentPanel> ElementsInstrumentPanel::Create(cockpit::PanelType type, std::weak_ptr<el::view> view, std::weak_ptr<IDataController> controller, InstrumentPanelWidgets widgets)
{
    auto self = std::make_shared<ElementsInstrumentPanel>(type, std::move(view), std::move(controller), std::move(widgets), EnsurePrivate{});
    self->InitSubscription();
    return self;
}

void ElementsInstrumentPanel::SetDataReady(bool ready)
{
    std::weak_ptr ref = std::static_pointer_cast<ElementsInstrumentPanel>(shared_from_this());

    PostToUi([ref, ready] {
        if (auto self = ref.lock()) {
            auto& deck = self->mWidgets.overlayDeck;
            if (!deck || deck->size() < 2) return;
            deck->select(ready ? 1 : 0);
            if (auto v = self->mView.lock()) {
                v->layout();
                v->refresh();
            }
        }
    });
}

void ElementsInstrumentPanel::PostToUi(std::function<void()> fn)
{
    if (auto v = mView.lock())
        v->post(std::move(fn));
}

void ElementsInstrumentPanel::OnInstrumentsReady(std::vector<std::string> symbols)
{
    if (!mWidgets.SetInstruments) return;

    std::weak_ptr ref = shared_from_this();

    mWidgets.SetInstruments(symbols, [ref](std::string sym) {
        if (auto self = ref.lock())
            self->SelectSymbol(std::move(sym));
    });

    if (auto v = mView.lock()) {
        v->layout();
        v->refresh();
    }
}

void ElementsInstrumentPanel::OnSymbolSelected(const std::string& symbol)
{
    if (mWidgets.SetTitle)
        mWidgets.SetTitle(symbol);

    if (Type() == cockpit::PanelType::MarketGraph && mWidgets.workArea) {
        mScratchPanel = PopulateMarketGraph(mView, mController, symbol, mWidgets.workArea);
    }

    if (auto v = mView.lock()) {
        v->layout();
        v->refresh();
    }
}

void ElementsInstrumentPanel::Update()
{
    std::weak_ptr<ScratchPanel> weak = mScratchPanel;
    PostToUi([weak]{ if (auto p = weak.lock()) p->Update(); });
}

} // namespace scratcher::elements
