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

#ifndef DATA_COLLECTOR_HPP
#define DATA_COLLECTOR_HPP

#include <string>
#include <memory>
#include <functional>
#include <boost/container/flat_map.hpp>

#include "data_controller.hpp"
#include "orderbook.hpp"
#include "scheduler.hpp"
#include "bybit/entities/response.hpp"
#include "bybit/entities/instrument.hpp"
#include "bybit/entities/public_trade.hpp"
#include "bybit/entities/order.hpp"
#include "bybit/entities/trade.hpp"
#include "bybit/entities/wallet.hpp"
#include "connection_context.hpp"
#include "http_query.hpp"
#include "websocket.hpp"
#include "datahub/data_provider.hpp"
#include "exchange_config.hpp"

namespace scratcher {
namespace bybit {

using scratcher::subscription_id;

class ByBitDataManager : public IDataController, public std::enable_shared_from_this<ByBitDataManager>
{
public:
    using instrument_provider_type = datahub::data_sink<datahub::data_model<InstrumentInfo, &InstrumentInfo::symbol>>;
    using pubtrade_provider_type   = datahub::data_sink<datahub::data_model<PublicTrade, &PublicTrade::execId>>;
    using orderbook_provider_type  = datahub::data_sink<OrderBook>;
    using order_provider_type      = datahub::data_sink<datahub::data_model<Order, &Order::orderId>>;
    using trade_provider_type      = datahub::data_sink<datahub::data_model<Trade, &Trade::execId>>;

private:
    static const std::string BYBIT;

    std::shared_ptr<connect::context>          m_context;
    std::shared_ptr<SQLite::Database>          m_db;
    std::shared_ptr<IExchangeConfig>             m_config;

    std::shared_ptr<instrument_provider_type>  m_instrument_provider;
    std::shared_ptr<connect::http_query>       m_instruments_query;
    std::shared_ptr<pubtrade_provider_type>    m_pubtrade_provider;
    std::shared_ptr<orderbook_provider_type>   m_orderbook_provider;
    std::shared_ptr<order_provider_type>       m_order_provider;
    std::shared_ptr<trade_provider_type>       m_trade_provider;

    std::shared_ptr<connect::websock_connection> m_public_stream;
    std::shared_ptr<connect::websock_connection> m_private_stream;

    size_t m_next_sub_id = 1;

    using instrument_handler = std::function<void(const std::deque<InstrumentInfo>&)>;
    using trade_handler = std::function<void(const std::deque<PublicTrade>&)>;
    using orderbook_handler = std::function<void(const std::vector<OrderBookLevel>&)>;

    boost::container::flat_map<subscription_id, instrument_handler> m_instrument_handlers;
    boost::container::flat_map<std::string, boost::container::flat_map<subscription_id, trade_handler>> m_trade_subs;
    boost::container::flat_map<std::string, boost::container::flat_map<subscription_id, orderbook_handler>> m_orderbook_subs;

    // symbol -> subscription_id reverse lookup (for unsubscribe)
    boost::container::flat_map<subscription_id, std::string> m_trade_sub_symbol;
    boost::container::flat_map<subscription_id, std::string> m_orderbook_sub_symbol;

    void SetupInstrumentDataSource();
    void SetupPublicDataSource();
    void SetupPrivateDataSource();

    struct ensure_private {};
public:
    ByBitDataManager(std::shared_ptr<scheduler> scheduler, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db, ensure_private);
    ~ByBitDataManager() override = default;

    static std::shared_ptr<ByBitDataManager> Create(std::shared_ptr<scheduler> scheduler, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db);

    const std::string& Name() const override { return BYBIT; }

    void HandleError(std::exception_ptr eptr);

    subscription_id SubscribeInstrumentList(std::function<void(const std::deque<InstrumentInfo>&)> handler) override;
    void UnsubscribeInstrumentList(subscription_id id) override;

    subscription_id SubscribePublicTrades(std::string symbol, std::function<void(const std::deque<PublicTrade>&)> handler) override;
    void UnsubscribePublicTrades(subscription_id id) override;

    subscription_id SubscribeOrderBook(std::string symbol, std::function<void(const std::vector<OrderBookLevel>&)> handler) override;
    void UnsubscribeOrderBook(subscription_id id) override;

    void SubscribeOrders(std::function<void(const std::deque<Order>&)> handler) override
    { m_order_provider->subscribe(std::move(handler)); }

    void SubscribeTrades(std::function<void(const std::deque<Trade>&)> handler) override
    { m_trade_provider->subscribe(std::move(handler)); }

    void SubscribeWallet(std::function<void(const WalletBalance&)> handler) override
    { /* TODO: WalletBalance contains nested deque<CoinBalance>, not yet supported by data_model */ }

    void PlaceOrder(OrderRequest request, std::function<void(std::string orderId)> callback) override;
    void CancelOrder(const std::string& orderId, const std::string& symbol) override;
};

} // scratcher::bybit
}

#endif //DATA_COLLECTOR_HPP
