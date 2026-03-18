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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ranges>

#include <glaze/glaze.hpp>
#include <openssl/hmac.h>
#include <openssl/evp.h>

#include "data_manager.hpp"
#include "datahub/data_sink.hpp"
#include "entities/orderbookdata.hpp"
#include "common/hex.hpp"

namespace scratcher::bybit {
namespace {
    constexpr auto STREAM_PUBLIC_SPOT = "/v5/public/spot";
    constexpr auto STREAM_PRIVATE     = "/v5/private";
    constexpr auto API_INSTRUMENTS    = "/v5/market/instruments-info?category=spot";
    constexpr auto API_RECENT_TRADE   = "/v5/market/recent-trade?category=spot&limit=60";

    std::string ping_message(size_t counter)
    {
        std::ostringstream buf;
        buf << R"({"req_id":")" << counter << R"(","op":"ping"})";
        return buf.str();
    }

    std::string subscribe_message(const std::string& topic)
    {
        return R"({"op":"subscribe","args":[")" + topic + R"("]})";
    }

    std::string unsubscribe_message(const std::string& topic)
    {
        return R"({"op":"unsubscribe","args":[")" + topic + R"("]})";
    }

    std::string extract_symbol(const std::string& topic)
    {
        auto pos = topic.rfind('.');
        return (pos != std::string::npos) ? topic.substr(pos + 1) : topic;
    }

    std::string current_timestamp_ms()
    {
        auto now = std::chrono::system_clock::now();
        return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    std::string hmac_sha256(const std::string& key, const std::string& data)
    {
        unsigned char result[EVP_MAX_MD_SIZE];
        unsigned int result_len = 0;
        HMAC(EVP_sha256(), key.data(), static_cast<int>(key.size()), reinterpret_cast<const unsigned char*>(data.data()), data.size(), result, &result_len);
        return hex(std::span(result, result_len));
    }

    connect::http_headers sign_rest_request(const std::string& api_key, const std::string& api_secret, const std::string& recv_window, const std::string& payload)
    {
        std::string timestamp = current_timestamp_ms();
        std::string signature = hmac_sha256(api_secret, timestamp + api_key + recv_window + payload);
        return {
            {"X-BAPI-API-KEY",     api_key},
            {"X-BAPI-TIMESTAMP",   timestamp},
            {"X-BAPI-SIGN",        signature},
            {"X-BAPI-RECV-WINDOW", recv_window}
        };
    }

    std::string ws_auth_message(const std::string& api_key, const std::string& api_secret)
    {
        auto now        = std::chrono::system_clock::now();
        auto expires_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() + 10000;
        std::string expires   = std::to_string(expires_ms);
        std::string signature = hmac_sha256(api_secret, "GET/realtime" + expires);
        std::ostringstream msg;
        msg << R"({"op":"auth","args":[")" << api_key << R"(",)" << expires << R"(,")" << signature << R"("]})";
        return msg.str();
    }

} // anonymous namespace

// Glaze-reflected types must live at namespace scope
struct PlaceOrderResult   { std::string orderId; };
struct PlaceOrderResponse { int retCode{0}; PlaceOrderResult result; };
struct CancelOrderRequest { std::string category; std::string symbol; std::string orderId; };

const std::string ByBitDataManager::BYBIT = "ByBit";

ByBitDataManager::ByBitDataManager(std::shared_ptr<scheduler> scheduler, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db, ensure_private)
    : m_context(connect::context::create(scheduler->io()))
    , m_db(std::move(db))
    , m_config(std::move(config))
{
}

std::shared_ptr<ByBitDataManager> ByBitDataManager::Create(std::shared_ptr<scheduler> scheduler, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db)
{
    auto self = std::make_shared<ByBitDataManager>(scheduler, std::move(config), std::move(db), ensure_private{});
    std::weak_ptr<ByBitDataManager> ref = self;

    auto error_cb = [ref](std::exception_ptr e){ if (auto s = ref.lock()) s->HandleError(e); };

    self->m_instrument_provider = datahub::make_data_sink<InstrumentInfo, &InstrumentInfo::symbol>(self->m_db, error_cb);
    self->m_pubtrade_provider = datahub::make_data_sink<PublicTrade, &PublicTrade::execId>(self->m_db, error_cb);

    self->m_orderbook_provider = datahub::make_data_sink(OrderBook::Create(), error_cb);

    self->m_order_provider = datahub::make_data_sink<Order, &Order::orderId>(self->m_db, error_cb);
    self->m_trade_provider = datahub::make_data_sink<Trade, &Trade::execId>(self->m_db, error_cb);

    self->SetupInstrumentDataSource();
    self->SetupPublicDataSource();
    self->SetupPrivateDataSource();

    return self;
}

void ByBitDataManager::HandleError(std::exception_ptr eptr)
{
    try {
        std::rethrow_exception(eptr);
    } catch (const std::exception& ex) {
        std::cerr << "ByBit data error: " << ex.what() << std::endl;
    }
}

void ByBitDataManager::SetupInstrumentDataSource()
{
    std::string url = "https://" + m_config->HttpHost() + ":" + m_config->HttpPort() + API_INSTRUMENTS;
    std::clog << "setupInstrumentDataSource: " << url << std::endl;

    auto entity_acceptor = m_instrument_provider->data_acceptor<std::deque<InstrumentInfoAPI>>();
    auto ref = weak_from_this();

    auto resp_adapter = datahub::make_data_adapter<ApiResponse<ListResult<InstrumentInfoAPI>>>(
        [entity_acceptor, ref](ApiResponse<ListResult<InstrumentInfoAPI>>&& response) mutable {
            std::clog << "Received " << response.result.list.size() << " instruments from server" << std::endl;
            std::deque<InstrumentInfo> instruments;
            for (const auto& api_item : response.result.list)
                instruments.emplace_back(static_cast<InstrumentInfo>(api_item));
            entity_acceptor(std::move(response.result.list));
            if (auto self = ref.lock()) {
                for (const auto& [id, handler] : self->m_instrument_handlers)
                    handler(instruments);
            }
        }
    );

    auto dispatcher = datahub::make_data_dispatcher(m_context->io().get_executor(), std::move(resp_adapter));

    m_instruments_query = connect::http_query::create(m_context, url, std::move(dispatcher),
        [ref](std::exception_ptr e) {
            if (auto self = ref.lock())
                self->HandleError(e);
        }
    );
}

void ByBitDataManager::SetupPublicDataSource()
{
    auto ref = weak_from_this();
    auto error_cb = [ref](std::exception_ptr e){ if (auto s = ref.lock()) s->HandleError(e); };

    m_public_stream = connect::websock_connection::create(m_context,
        "wss://" + m_config->StreamHost() + ":" + m_config->StreamPort() + STREAM_PUBLIC_SPOT,
        datahub::make_data_dispatcher(m_context->io().get_executor(),

            datahub::make_data_adapter<WsApiPayload<std::deque<WsPublicTrade>>>([ref](WsApiPayload<std::deque<WsPublicTrade>>&& payload) mutable {
                if (auto self = ref.lock()) {
                    auto symbol = extract_symbol(payload.topic);
                    std::deque<PublicTrade> trades(payload.data.begin(), payload.data.end());
                    auto acceptor = self->m_pubtrade_provider->template data_acceptor<std::deque<WsPublicTrade>>();
                    acceptor(std::move(payload.data));
                    if (auto it = self->m_trade_subs.find(symbol); it != self->m_trade_subs.end()) {
                        for (const auto& [id, handler] : it->second)
                            handler(trades);
                    }
                }
            }),

            datahub::make_data_adapter<WsApiPayload<OrderBookData>>(
                [ref](WsApiPayload<OrderBookData>&& payload) mutable {
                    if (auto self = ref.lock()) {
                        auto symbol = extract_symbol(payload.topic);

                        // Negate ask sizes: wire format has positive sizes, domain uses negative for asks
                        for (auto& ask : payload.data.a)
                            ask.size = -ask.size;

                        std::deque<scratcher::OrderBookLevel> levels;
                        std::ranges::move(payload.data.b, std::back_inserter(levels));
                        std::ranges::move(payload.data.a, std::back_inserter(levels));

                        auto book = self->m_orderbook_provider->model();
                        if (payload.type == "snapshot") book->Snapshot(levels);
                        else book->Merge(levels);

                        if (auto it = self->m_orderbook_subs.find(symbol); it != self->m_orderbook_subs.end()) {
                            const auto& merged = book->Levels();
                            for (const auto& [id, handler] : it->second)
                                handler(merged);
                        }
                    }
                })),

        error_cb);
    m_public_stream->set_heartbeat(std::chrono::seconds(20), ping_message);
}

void ByBitDataManager::SetupPrivateDataSource()
{
    if (!m_config->HasApiCredentials()) {
        std::clog << "No API credentials, skipping private stream" << std::endl;
        return;
    }

    auto ref = weak_from_this();
    auto error_cb = [ref](std::exception_ptr e){ if (auto s = ref.lock()) s->HandleError(e); };

    m_private_stream = connect::websock_connection::create(m_context,
        "wss://" + m_config->StreamHost() + ":" + m_config->StreamPort() + STREAM_PRIVATE,
        datahub::make_data_dispatcher(m_context->io().get_executor(),

            datahub::make_data_adapter<WsApiPayload<std::deque<Order>>>([ref](WsApiPayload<std::deque<Order>>&& payload) mutable {
                if (auto self = ref.lock()) {
                    auto acceptor = self->m_order_provider->template data_acceptor<std::deque<Order>>();
                    acceptor(std::move(payload.data));
                }
            }),

            datahub::make_data_adapter<WsApiPayload<std::deque<Trade>>>([ref](WsApiPayload<std::deque<Trade>>&& payload) mutable {
                if (auto self = ref.lock()) {
                    auto acceptor = self->m_trade_provider->template data_acceptor<std::deque<Trade>>();
                    acceptor(std::move(payload.data));
                }
            })),

        error_cb);

    (*m_private_stream)(ws_auth_message(m_config->ApiKey(), m_config->ApiSecret()));
    (*m_private_stream)(subscribe_message("order"));
    (*m_private_stream)(subscribe_message("execution"));
    m_private_stream->set_heartbeat(std::chrono::seconds(20), ping_message);
}

// ─── IDataController subscriptions ───────────────────────────────────────────

subscription_id ByBitDataManager::SubscribeInstrumentList(std::function<void(const std::deque<InstrumentInfo>&)> handler)
{
    auto id = m_next_sub_id++;
    m_instrument_handlers.emplace(id, std::move(handler));
    (*m_instruments_query)();
    return id;
}

void ByBitDataManager::UnsubscribeInstrumentList(subscription_id id)
{
    m_instrument_handlers.erase(id);
}

subscription_id ByBitDataManager::SubscribePublicTrades(std::string symbol, std::function<void(const std::deque<PublicTrade>&)> handler)
{
    auto id = m_next_sub_id++;
    m_trade_subs[symbol].emplace(id, std::move(handler));
    m_trade_sub_symbol.emplace(id, symbol);
    (*m_public_stream)(subscribe_message("publicTrade." + symbol));
    return id;
}

void ByBitDataManager::UnsubscribePublicTrades(subscription_id id)
{
    if (auto sym_it = m_trade_sub_symbol.find(id); sym_it != m_trade_sub_symbol.end()) {
        auto& sym_map = m_trade_subs[sym_it->second];
        sym_map.erase(id);
        if (sym_map.empty()) {
            (*m_public_stream)(unsubscribe_message("publicTrade." + sym_it->second));
            m_trade_subs.erase(sym_it->second);
        }
        m_trade_sub_symbol.erase(sym_it);
    }
}

subscription_id ByBitDataManager::SubscribeOrderBook(std::string symbol, std::function<void(const std::vector<OrderBookLevel>&)> handler)
{
    auto id = m_next_sub_id++;
    m_orderbook_subs[symbol].emplace(id, std::move(handler));
    m_orderbook_sub_symbol.emplace(id, symbol);
    (*m_public_stream)(subscribe_message("orderbook.50." + symbol));
    return id;
}

void ByBitDataManager::UnsubscribeOrderBook(subscription_id id)
{
    if (auto sym_it = m_orderbook_sub_symbol.find(id); sym_it != m_orderbook_sub_symbol.end()) {
        auto& sym_map = m_orderbook_subs[sym_it->second];
        sym_map.erase(id);
        if (sym_map.empty()) {
            (*m_public_stream)(unsubscribe_message("orderbook.50." + sym_it->second));
            m_orderbook_subs.erase(sym_it->second);
        }
        m_orderbook_sub_symbol.erase(sym_it);
    }
}

// ─── Order management ─────────────────────────────────────────────────────────

void ByBitDataManager::PlaceOrder(OrderRequest request, std::function<void(std::string orderId)> callback)
{
    if (!m_config->HasApiCredentials()) {
        std::cerr << "PlaceOrder: no API credentials configured" << std::endl;
        return;
    }

    std::string body  = glz::write_json(request).value_or("{}");
    auto headers      = sign_rest_request(m_config->ApiKey(), m_config->ApiSecret(), "5000", body);
    auto ref          = weak_from_this();

    auto query = connect::http_query::create(m_context, boost::beast::http::verb::post,
        "https://" + m_config->HttpHost() + ":" + m_config->HttpPort() + "/v5/order/create",
        [callback](std::string&& response_json) {
            PlaceOrderResponse resp;
            if (!glz::read<glz::opts{.error_on_unknown_keys = false}>(resp, response_json) && resp.retCode == 0) {
                if (callback) callback(std::move(resp.result.orderId));
            } else {
                std::cerr << "PlaceOrder failed: " << response_json << std::endl;
            }
        },
        [ref](std::exception_ptr e){ if (auto s = ref.lock()) s->HandleError(e); });
    (*query)({}, std::move(headers), std::move(body));
}

void ByBitDataManager::CancelOrder(const std::string& orderId, const std::string& symbol)
{
    if (!m_config->HasApiCredentials()) {
        std::cerr << "CancelOrder: no API credentials configured" << std::endl;
        return;
    }

    CancelOrderRequest req{.category = "spot", .symbol = symbol, .orderId = orderId};
    std::string body  = glz::write_json(req).value_or("{}");
    auto headers      = sign_rest_request(m_config->ApiKey(), m_config->ApiSecret(), "5000", body);
    auto ref          = weak_from_this();

    auto query = connect::http_query::create(m_context, boost::beast::http::verb::post,
        "https://" + m_config->HttpHost() + ":" + m_config->HttpPort() + "/v5/order/cancel",
        [](std::string&& response_json) { std::clog << "CancelOrder response: " << response_json << std::endl; },
        [ref](std::exception_ptr e){ if (auto s = ref.lock()) s->HandleError(e); });
    (*query)({}, std::move(headers), std::move(body));
}

} // scratcher::bybit
