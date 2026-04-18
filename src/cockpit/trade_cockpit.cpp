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

#include "trade_cockpit.hpp"

#include <chrono>
#include <optional>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/system/error_code.hpp>

namespace scratcher::cockpit {

TradeCockpit::TradeCockpit(std::shared_ptr<scheduler> sched, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db, EnsurePrivate)
    : mScheduler(std::move(sched))
{
    mDataManager = bybit::ByBitDataManager::Create(mScheduler, config, db);
}

std::shared_ptr<TradeCockpit> TradeCockpit::Create(std::shared_ptr<scheduler> sched, std::shared_ptr<IExchangeConfig> config, std::shared_ptr<SQLite::Database> db)
{
    auto self = std::make_shared<TradeCockpit>(std::move(sched), std::move(config), std::move(db), EnsurePrivate{});

    self->mInstrumentSub = datahub::make_data_subscription<std::deque<bybit::InstrumentInfo>>(
        [weak = std::weak_ptr(self)](auto update) {
            if (auto s = weak.lock())
                s->OnInstrumentsLoaded();
        });

    self->mDataManager->SubscribeInstrumentList(self->mInstrumentSub);

    boost::asio::co_spawn(self->mScheduler->io(), coUpdate(self), boost::asio::detached);

    return self;
}

boost::asio::awaitable<void> TradeCockpit::coUpdate(std::weak_ptr<TradeCockpit> ref)
{
    using namespace std::chrono;
    while (true) {
        try {
            std::optional<boost::asio::steady_timer> timer;
            if (auto self = ref.lock()) {
                for (auto& [pid, panel] : self->mPanels) panel->Update();
                timer.emplace(self->mScheduler->io(), milliseconds(1000));
            } else {
                break;
            }
            if (timer) {
                timer->expires_after(milliseconds(100));
                co_await timer->async_wait(boost::asio::use_awaitable);
            } else {
                break;
            }
        }
        catch (boost::system::error_code& e) {
            if (e.value() == boost::asio::error::operation_aborted) break;
        }
        catch (const std::exception&) {}
    }
}

panel_id TradeCockpit::RegisterPanel(std::shared_ptr<ContentPanel> panel)
{
    panel_id pid = mNextPanelId++;

    if (mDataReady)
        panel->SetDataReady(true);

    mPanels[pid] = std::move(panel);
    return pid;
}

void TradeCockpit::UnregisterPanel(panel_id pid)
{
    mPanels.erase(pid);
}

void TradeCockpit::OnInstrumentsLoaded()
{
    const auto& snapshot = mDataManager->getInstrumentsFeed().get_snapshot();

    mInstruments.clear();
    mInstruments.reserve(snapshot.size());
    for (const auto& inst : snapshot) {
        mInstruments.emplace_back(inst.symbol);
    }

    mDataReady = true;
    for (auto& [pid, panel] : mPanels)
        panel->SetDataReady(true);
}

} // namespace scratcher::cockpit
