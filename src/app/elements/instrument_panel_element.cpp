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

#include "instrument_panel_element.hpp"

#include <span>
#include <utility>

#include <elements.hpp>

#include "pixel_buffer_element.hpp"

namespace scratcher::elements {

namespace el = cycfi::elements;

InstrumentPanelElement::InstrumentPanelElement(cockpit::PanelType type, seconds candle_period, uint32_t candle_width_pixels,
                                                  std::weak_ptr<el::view> view, InstrumentPanelWidgets widgets, EnsurePrivate)
    : InstrumentPanel(type, candle_period, candle_width_pixels)
    , mView(std::move(view))
    , mWidgets(std::move(widgets))
{}

InstrumentPanelElement::~InstrumentPanelElement() = default;

std::shared_ptr<InstrumentPanelElement> InstrumentPanelElement::Create(cockpit::PanelType type, seconds candle_period, uint32_t candle_width_pixels,
                                                                          std::weak_ptr<el::view> view, InstrumentPanelWidgets widgets)
{
    auto self = std::make_shared<InstrumentPanelElement>(type, candle_period, candle_width_pixels, std::move(view), std::move(widgets), EnsurePrivate{});

    self->mPixelBuffer = std::make_shared<PixelBufferElement>();

    std::weak_ptr<InstrumentPanel> ref = self;
    self->mPixelBuffer->SetOnResize([ref](uint32_t* buffer, uint32_t stride, uint32_t w, uint32_t h) {
        if (auto p = ref.lock()) {
            p->SetTarget(std::span<uint32_t>{buffer, static_cast<size_t>(stride) * h}, stride, w, h);
            p->OnSize(static_cast<int>(w), static_cast<int>(h));
        }
    });
    self->mPixelBuffer->SetOnRender([ref] {
        if (auto p = ref.lock()) {
            // Circuit A: try-lock + frame-throttle gate. If a worker is mid-Update
            // or the throttle window is still open, OnUpdate() returns immediately
            // and Render() draws the previously-published scene.
            p->OnUpdate();
            return p->Render();
        }
        return cockpit::PixelRect{};
    });

    if (self->mWidgets.workArea) {
        self->mWidgets.workArea->push_back(el::share(el::hold(self->mPixelBuffer)));
        self->mWidgets.workArea->select(0);
    }

    return self;
}

void InstrumentPanelElement::Update()
{
    // Worker-safe by construction now: InstrumentPanel::Update() takes the data lock
    // and recalculates in the calling thread. Refresh() then schedules the UI redraw
    // via view::refresh(), which Cycfi makes thread-safe through asio::post.
    InstrumentPanel::Update();
    Refresh();
}

void InstrumentPanelElement::Refresh()
{
    if (auto v = mView.lock())
        v->refresh();
}

void InstrumentPanelElement::PostToUi(std::function<void()> fn)
{
    if (auto v = mView.lock())
        v->post(std::move(fn));
}

void InstrumentPanelElement::OnSymbolChanged(const std::string& symbol)
{
    auto ref = weak_from_this();
    PostToUi([ref, symbol] {
        auto base = ref.lock();
        if (!base) return;
        auto self = std::static_pointer_cast<InstrumentPanelElement>(base);
        if (self->mWidgets.SetTitle)
            self->mWidgets.SetTitle(symbol);
        if (auto v = self->mView.lock()) {
            v->layout();
            v->refresh();
        }
    });
}

void InstrumentPanelElement::OnInstrumentListChanged(const std::vector<std::string>& symbols)
{
    auto ref = weak_from_this();
    PostToUi([ref, symbols] {
        auto base = ref.lock();
        if (!base) return;
        auto self = std::static_pointer_cast<InstrumentPanelElement>(base);
        if (!self->mWidgets.SetInstruments) return;

        std::weak_ptr<InstrumentPanelElement> ref = self;
        self->mWidgets.SetInstruments(symbols, [ref](std::string sym) {
            if (auto s = ref.lock())
                s->EmitUserSymbolSelection(std::move(sym));
        });

        if (auto v = self->mView.lock()) {
            v->layout();
            v->refresh();
        }
    });
}

void InstrumentPanelElement::OnInstrumentInfoChanged(const std::optional<bybit::InstrumentInfo>& /*info*/)
{
    auto ref = weak_from_this();
    PostToUi([ref] {
        auto base = ref.lock();
        if (!base) return;
        auto self = std::static_pointer_cast<InstrumentPanelElement>(base);
        if (auto v = self->mView.lock()) {
            v->layout();
            v->refresh();
        }
    });
}

} // namespace scratcher::elements
