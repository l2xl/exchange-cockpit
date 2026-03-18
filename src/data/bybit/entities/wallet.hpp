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

namespace scratcher::bybit {

struct CoinBalance {
    std::string coin;
    std::optional<std::string> equity;
    std::optional<std::string> usdValue;
    std::optional<std::string> walletBalance;
    std::optional<std::string> availableToWithdraw;
    std::optional<std::string> unrealisedPnl;
    std::optional<std::string> cumRealisedPnl;
};

struct WalletBalance {
    AccountType accountType;
    std::optional<std::string> totalEquity;
    std::optional<std::string> totalWalletBalance;
    std::optional<std::string> totalAvailableBalance;
    std::optional<std::string> totalPerpUPL;
    std::deque<CoinBalance> coin;
};

} // namespace scratcher::bybit

#endif // BYBIT_WALLET_HPP
