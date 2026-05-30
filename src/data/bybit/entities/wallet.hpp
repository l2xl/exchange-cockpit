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

#ifndef BYBIT_WALLET_HPP
#define BYBIT_WALLET_HPP

#include <string>
#include <deque>
#include <optional>
#include "enums.hpp"
#include "currency.hpp"

namespace scratcher::bybit {

using scratcher::currency;

// PnL fields are signed (currency<int64_t>); balances/equity are non-negative.
struct CoinBalance {
    std::string coin;
    std::optional<currency<uint64_t>> equity;
    std::optional<currency<uint64_t>> usdValue;
    std::optional<currency<uint64_t>> walletBalance;
    std::optional<currency<uint64_t>> availableToWithdraw;
    std::optional<currency<int64_t>> unrealisedPnl;
    std::optional<currency<int64_t>> cumRealisedPnl;
};

struct WalletBalance {
    AccountType accountType;
    std::optional<currency<uint64_t>> totalEquity;
    std::optional<currency<uint64_t>> totalWalletBalance;
    std::optional<currency<uint64_t>> totalAvailableBalance;
    std::optional<currency<int64_t>> totalPerpUPL;
    std::deque<CoinBalance> coin;
};

} // namespace scratcher::bybit

#endif // BYBIT_WALLET_HPP
