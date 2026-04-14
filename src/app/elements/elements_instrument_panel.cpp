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

namespace scratcher::elements {

namespace el = cycfi::elements;

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
    if (auto v = mView.lock()) {
        v->layout();
        v->refresh();
    }
}

} // namespace scratcher::elements
