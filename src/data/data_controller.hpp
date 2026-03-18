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

#ifndef DATA_PROVIDER_HPP
#define DATA_PROVIDER_HPP

#include <functional>
#include <deque>
#include <string>
#include <cstddef>

#include "orderbook.hpp"
#include "bybit/entities/instrument.hpp"
#include "bybit/entities/public_trade.hpp"
#include "bybit/entities/order.hpp"
#include "bybit/entities/trade.hpp"
#include "bybit/entities/wallet.hpp"

namespace scratcher {

using subscription_id = size_t;

struct IDataController
{
    virtual ~IDataController() = default;
    virtual const std::string& Name() const = 0;

    virtual subscription_id SubscribeInstrumentList(std::function<void(const std::deque<bybit::InstrumentInfo>&)> handler) = 0;
    virtual void UnsubscribeInstrumentList(subscription_id id) = 0;

    virtual subscription_id SubscribePublicTrades(std::string symbol, std::function<void(const std::deque<bybit::PublicTrade>&)> handler) = 0;
    virtual void UnsubscribePublicTrades(subscription_id id) = 0;

    virtual subscription_id SubscribeOrderBook(std::string symbol, std::function<void(const std::vector<OrderBookLevel>&)> handler) = 0;
    virtual void UnsubscribeOrderBook(subscription_id id) = 0;

    virtual void SubscribeOrders(std::function<void(const std::deque<bybit::Order>&)> handler) = 0;
    virtual void SubscribeTrades(std::function<void(const std::deque<bybit::Trade>&)> handler) = 0;
    virtual void SubscribeWallet(std::function<void(const bybit::WalletBalance&)> handler) = 0;

    virtual void PlaceOrder(bybit::OrderRequest request, std::function<void(std::string orderId)> callback) = 0;
    virtual void CancelOrder(const std::string& orderId, const std::string& symbol) = 0;
};

} // scratcher

#endif //DATA_PROVIDER_HPP
