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

    return self;
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
