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

#include <string>
#include <vector>
#include <memory>
#include <deque>

#include <boost/asio/awaitable.hpp>
#include <boost/container/flat_map.hpp>

#include "scheduler.hpp"
#include "bybit/data_manager.hpp"
#include "content_panel.hpp"

namespace SQLite {
class Database;
}

namespace scratcher::cockpit {

class TradeCockpit : public std::enable_shared_from_this<TradeCockpit>
{
private:
    std::shared_ptr<scheduler> mScheduler;
    std::shared_ptr<IDataController> mDataManager;
    std::shared_ptr<datahub::data_subscription<std::deque<bybit::InstrumentInfo>>> mInstrumentSub;

    std::vector<std::string> mInstruments;
    boost::container::flat_map<panel_id, std::shared_ptr<ContentPanel>> mPanels;
    bool mDataReady = false;
    panel_id mNextPanelId = 1;

    struct EnsurePrivate {};

    void OnInstrumentsLoaded();

    static boost::asio::awaitable<void> coUpdate(std::weak_ptr<TradeCockpit> ref);

public:
    TradeCockpit(std::shared_ptr<scheduler> sched, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db, EnsurePrivate);

    static std::shared_ptr<TradeCockpit> Create(std::shared_ptr<scheduler> sched, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db);

    panel_id RegisterPanel(std::shared_ptr<ContentPanel> panel);
    void UnregisterPanel(panel_id pid);

    std::shared_ptr<IDataController> GetDataController() const { return mDataManager; }
};

} // namespace scratcher::cockpit
