// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)
// -----BEGIN PGP PUBLIC KEY BLOCK-----
// mDMEYdxcVRYJKwYBBAHaRw8BAQdAfacBVThCP5QDPEgSbSIudtpJS4Y4Imm5dzaN
// lM1HTem0IkwyIFhsIChsMnhsKSA8bDJ4bEBwcm90b21tYWlsLmNvbT6IkAQTFggA
// OBYhBKRCfUyWnduCkisNl+WRcOaCK79JBQJh3FxVAhsDBQsJCAcCBhUKCQgLAgQW
// AgMBAh4BAheAAAoJEOWRcOaCK79JDl8A/0/AjYVbAURZJXP3tHRgZyYyN9txT6mW
// 0bYCcOf0rZ4NAQDoFX4dytPDvcjV7ovSQJ6dzvIoaRbKWGbHRCufrm5QBA==
// =KKu7
// -----END PGP PUBLIC KEY BLOCK-----

#ifndef SCRATCHER_ORDERBOOK_LEVEL_HPP
#define SCRATCHER_ORDERBOOK_LEVEL_HPP

#include <glaze/glaze.hpp>

#include "currency.hpp"

namespace scratcher {

// Domain orderbook level: positive size = bid, negative size = ask, zero = remove
struct OrderBookLevel {
    currency<int64_t> price;
    currency<int64_t> size;
};

} // namespace scratcher

// Glaze: currency<int64_t> reads/writes as a JSON quoted string
template<>
struct glz::meta<scratcher::currency<int64_t>> {
    static constexpr auto value = custom<
        [](scratcher::currency<int64_t>& c, const std::string& s) { c = scratcher::currency<int64_t>(s); },
        [](const scratcher::currency<int64_t>& c) -> std::string { return c.to_string(); }
    >;
};

// Glaze: OrderBookLevel is a JSON array ["price", "size"]
template<>
struct glz::meta<scratcher::OrderBookLevel> {
    using T = scratcher::OrderBookLevel;
    static constexpr auto value = array(&T::price, &T::size);
};

#endif // SCRATCHER_ORDERBOOK_LEVEL_HPP
