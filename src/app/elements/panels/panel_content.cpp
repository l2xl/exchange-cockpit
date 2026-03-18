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

#include "panel_content.hpp"

namespace scratcher::elements::panels {

namespace el = cycfi::elements;

namespace {

el::element_ptr make_table_header(std::initializer_list<std::string> columns)
{
    el::htile_composite row;
    for (const auto& col : columns) {
        row.push_back(el::share(
            el::hstretch(1.0,
                el::margin({4, 2, 4, 2},
                    el::label(col).font_size(11).font_color(dim_text())
                )
            )
        ));
    }
    return el::share(
        el::layer(
            el::htile(std::move(row)),
            el::box(table_header_bg())
        )
    );
}

el::element_ptr make_placeholder(const std::string& text)
{
    return el::share(
        el::layer(
            el::align_center_middle(
                el::label(text).font_size(14).font_color(dim_text())
            ),
            el::box(panel_bg_color())
        )
    );
}

} // anonymous namespace

el::element_ptr MakeMarketGraphContent()
{
    return el::share(
        el::layer(
            el::vtile(
                el::vstretch(0.85,
                    el::layer(
                        el::align_center_middle(
                            el::vtile(
                                el::label("Price Chart").font_size(18).font_color(bright_text()),
                                el::vspace(8),
                                el::label("Tick chart from public trade stream")
                                    .font_size(12).font_color(dim_text())
                            )
                        ),
                        el::box(panel_bg_color())
                    )
                ),
                el::hmin_size(1, el::vsize(1, el::box(el::rgba(60, 60, 65, 255)))),
                el::vstretch(0.15,
                    el::layer(
                        el::align_center_middle(
                            el::label("Volume").font_size(11).font_color(dim_text())
                        ),
                        el::box(el::rgba(25, 25, 30, 255))
                    )
                )
            ),
            el::box(panel_bg_color())
        )
    );
}

el::element_ptr MakeOrderBookContent()
{
    return el::share(
        el::vtile(
            el::hold(make_table_header({"Price", "Size", "Total"})),
            el::vstretch(0.5,
                el::layer(
                    el::align_center_middle(
                        el::label("Asks").font_size(14).font_color(ask_color())
                    ),
                    el::box(panel_bg_color())
                )
            ),
            el::layer(
                el::align_center(
                    el::margin({8, 4, 8, 4},
                        el::label("Spread: --").font_size(12).font_color(bright_text())
                    )
                ),
                el::box(el::rgba(35, 35, 40, 255))
            ),
            el::vstretch(0.5,
                el::layer(
                    el::align_center_middle(
                        el::label("Bids").font_size(14).font_color(bid_color())
                    ),
                    el::box(panel_bg_color())
                )
            )
        )
    );
}

el::element_ptr MakeOrdersContent()
{
    return el::share(
        el::vtile(
            el::hold(make_table_header({"Order ID", "Symbol", "Side", "Type", "Price", "Qty", "Status", "Time"})),
            el::vstretch(1.0,
                el::hold(make_placeholder("No orders"))
            )
        )
    );
}

el::element_ptr MakeTradeHistoryContent()
{
    return el::share(
        el::vtile(
            el::hold(make_table_header({"Exec ID", "Symbol", "Side", "Price", "Qty", "Fee", "Time"})),
            el::vstretch(1.0,
                el::hold(make_placeholder("No trade history"))
            )
        )
    );
}

el::element_ptr MakeNewOrderContent()
{
    auto buy_btn = el::share(
        el::momentary_button(
            el::margin({16, 8, 16, 8},
                el::label("BUY").font_color(el::rgba(255, 255, 255, 255))
            )
        )
    );

    auto sell_btn = el::share(
        el::momentary_button(
            el::margin({16, 8, 16, 8},
                el::label("SELL").font_color(el::rgba(255, 255, 255, 255))
            )
        )
    );

    return el::share(
        el::layer(
            el::margin({12, 8, 12, 8},
                el::vtile(
                    el::label("Symbol").font_size(11).font_color(dim_text()),
                    el::vspace(4),
                    el::label("BTCUSDT").font_size(14).font_color(bright_text()),
                    el::vspace(12),
                    el::label("Order Type").font_size(11).font_color(dim_text()),
                    el::vspace(4),
                    el::label("Limit").font_size(14).font_color(bright_text()),
                    el::vspace(12),
                    el::label("Price").font_size(11).font_color(dim_text()),
                    el::vspace(4),
                    el::label("--").font_size(14).font_color(bright_text()),
                    el::vspace(12),
                    el::label("Quantity").font_size(11).font_color(dim_text()),
                    el::vspace(4),
                    el::label("--").font_size(14).font_color(bright_text()),
                    el::vspace(16),
                    el::htile(
                        el::hstretch(1.0, el::hold(buy_btn)),
                        el::hspace(8),
                        el::hstretch(1.0, el::hold(sell_btn))
                    ),
                    el::vstretch(1.0, el::element{})
                )
            ),
            el::box(panel_bg_color())
        )
    );
}

el::element_ptr MakeTradeStatsContent()
{
    return el::share(
        el::layer(
            el::margin({12, 8, 12, 8},
                el::vtile(
                    el::label("Trade Statistics").font_size(16).font_color(bright_text()),
                    el::vspace(12),
                    el::htile(
                        el::vtile(
                            el::label("Total PnL").font_size(11).font_color(dim_text()),
                            el::label("--").font_size(18).font_color(bright_text())
                        ),
                        el::vtile(
                            el::label("Win Rate").font_size(11).font_color(dim_text()),
                            el::label("--").font_size(18).font_color(bright_text())
                        )
                    ),
                    el::vspace(12),
                    el::htile(
                        el::vtile(
                            el::label("Profit Factor").font_size(11).font_color(dim_text()),
                            el::label("--").font_size(18).font_color(bright_text())
                        ),
                        el::vtile(
                            el::label("Max Drawdown").font_size(11).font_color(dim_text()),
                            el::label("--").font_size(18).font_color(bright_text())
                        )
                    ),
                    el::vspace(16),
                    el::vstretch(1.0,
                        el::layer(
                            el::align_center_middle(
                                el::label("Equity Curve").font_size(14).font_color(dim_text())
                            ),
                            el::box(el::rgba(25, 25, 30, 255))
                        )
                    )
                )
            ),
            el::box(panel_bg_color())
        )
    );
}

el::element_ptr MakePositionsContent()
{
    return el::share(
        el::vtile(
            el::hold(make_table_header({"Symbol", "Side", "Size", "Entry", "Mark", "PnL", "Liq Price"})),
            el::vstretch(1.0,
                el::hold(make_placeholder("No open positions"))
            )
        )
    );
}

el::element_ptr MakeWatchlistContent()
{
    return el::share(
        el::vtile(
            el::hold(make_table_header({"Symbol", "Last Price", "24h Change", "Volume"})),
            el::vstretch(1.0,
                el::hold(make_placeholder("Loading instruments..."))
            )
        )
    );
}

} // namespace scratcher::elements::panels
