// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
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

#include <elements.hpp>
#include "content_panel.hpp"

namespace scratcher::elements::panels {

namespace el = cycfi::elements;

// Common UI colors for panels
inline el::color panel_bg_color() { return el::rgba(30, 30, 35, 255); }
inline el::color table_header_bg() { return el::rgba(45, 45, 50, 255); }
inline el::color bid_color() { return el::rgba(50, 180, 80, 255); }
inline el::color ask_color() { return el::rgba(220, 60, 60, 255); }
inline el::color dim_text() { return el::rgba(128, 128, 128, 255); }
inline el::color bright_text() { return el::rgba(220, 220, 220, 255); }
inline el::color accent_color() { return el::rgba(80, 140, 230, 255); }

// Panel content factory per PanelType
el::element_ptr MakeMarketGraphContent();
el::element_ptr MakeOrderBookContent();
el::element_ptr MakeOrdersContent();
el::element_ptr MakeTradeHistoryContent();
el::element_ptr MakeNewOrderContent();
el::element_ptr MakeTradeStatsContent();
el::element_ptr MakePositionsContent();
el::element_ptr MakeWatchlistContent();

} // namespace scratcher::elements::panels
