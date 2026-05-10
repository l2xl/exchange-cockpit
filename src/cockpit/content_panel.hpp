// Scratcher project
// Copyright (c) 2025 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)
// -----BEGIN PGP PUBLIC KEY BLOCK-----
//
// mDMEYdxcVRYJKwYBBAHaRw8BAQdAfacBVThCP5QDPEgSbSIudtpJS4Y4Imm5dzaN
// lM1HTem0IkwyIFhsIChsMnhsKSA8bDJ4bEBwcm90b21tYWlsLmNvbT6IkAQTFggA
// OBYhBKRCfUyWnduCkisNl+WRcOaCK79JBQJh3FxVAhsDBQsJCAcCBhUKCQgLAgQW
// AgMBAh4BAheAAAoJEOWRcOaCK79JDl8A/0/AjYVbAURZJXP3tHRgZyYyN9txT6mW
// 0bYCcOf0rZ4NAQDoFX4dytPDvcjV7ovSQJ6dzvIoaRbKWGbHRCufrm5QBA==
// =KKu7
// -----END PGP PUBLIC KEY BLOCK-----

#pragma once

#include <string>
#include <cstddef>

namespace scratcher::cockpit {

using panel_id = size_t;

enum class PanelType { Empty, MarketGraph, OrderBook, Orders, TradeHistory, NewOrder, TradeStats, Positions, Watchlist };

std::string PanelTypeName(PanelType type);

class ContentPanel
{
public:
    explicit ContentPanel(PanelType type) : mType(type) {}
    virtual ~ContentPanel() = default;

    PanelType Type() const { return mType; }

    // Worker-safe blocking update. Acquires the panel's data lock, recalculates
    // the vector model, releases. Subclasses that own a UI surface follow with
    // their own Refresh() to schedule a redraw.
    virtual void Update() = 0;

    // Cheap UI-redraw request. Thread-safe by contract; implementations must not
    // touch the data lock here.
    virtual void Refresh() = 0;

private:
    PanelType mType;
};

} // namespace scratcher::cockpit
