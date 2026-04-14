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

#include "instrument_panel.hpp"

namespace scratcher::cockpit {

InstrumentPanel::InstrumentPanel(PanelType type, std::weak_ptr<IDataController> controller)
    : ContentPanel(type), mController(std::move(controller))
{}

InstrumentPanel::~InstrumentPanel() = default;

void InstrumentPanel::InitSubscription()
{
    std::weak_ptr<InstrumentPanel> ref = shared_from_this();

    mListSub = datahub::make_data_subscription<std::deque<bybit::InstrumentInfo>>(
        [ref](auto /*update*/) {
            if (auto self = ref.lock()) {
                self->PostToUi([ref]() {
                    auto s = ref.lock();
                    if (!s) return;
                    auto ctl = s->mController.lock();
                    if (!ctl) return;
                    const auto& snapshot = ctl->getInstrumentsFeed().get_snapshot();
                    std::vector<std::string> symbols;
                    symbols.reserve(snapshot.size());
                    for (const auto& inst : snapshot)
                        symbols.emplace_back(inst.symbol);
                    s->OnInstrumentsReady(std::move(symbols));
                });
            }
        });

    if (auto ctl = mController.lock())
        ctl->SubscribeInstrumentList(mListSub);
}

void InstrumentPanel::SelectSymbol(std::string symbol)
{
    mSymbol = std::move(symbol);
    OnSymbolSelected(mSymbol);
}

} // namespace scratcher::cockpit
