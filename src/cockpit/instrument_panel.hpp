// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)
// -----BEGIN PGP PUBLIC KEY BLOCK-----
//
// mDMEYdxcVRYJKwYBBAHaRw8BAQdAfacBVThCP5QDPEgSbSIudtpJS4Y4Imm5dzaN
// lM1HTem0IkwyIFhsIChsMnhsKSA8bDJ4bEBwcm90b25tYWlsLmNvbT6IkAQTFggA
// OBYhBKRCfUyWnduCkisNl+WRcOaCK79JBQJh3FxVAhsDBQsJCAcCBhUKCQgLAgQW
// AgMBAh4BAheAAAoJEOWRcOaCK79JDl8A/0/AjYVbAURZJXP3tHRgZyYyN9txT6mW
// 0bYCcOf0rZ4NAQDoFX4dytPDvcjV7ovSQJ6dzvIoaRbKWGbHRCufrm5QBA==
// =KKu7
// -----END PGP PUBLIC KEY BLOCK-----

#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "content_panel.hpp"
#include "data_controller.hpp"
#include "bybit/entities/instrument.hpp"
#include "datahub/data_subscription.hpp"

namespace scratcher::cockpit {

class InstrumentPanel : public ContentPanel, public std::enable_shared_from_this<InstrumentPanel>
{
public:
    InstrumentPanel(PanelType type, std::weak_ptr<IDataController> controller);
    ~InstrumentPanel() override;

    void SelectSymbol(std::string symbol);
    const std::string& Symbol() const { return mSymbol; }

protected:
    void InitSubscription();

    virtual void PostToUi(std::function<void()> fn) = 0;
    virtual void OnInstrumentsReady(std::vector<std::string> symbols) = 0;
    virtual void OnSymbolSelected(const std::string& symbol) = 0;

    std::weak_ptr<IDataController> mController;

private:
    std::shared_ptr<datahub::data_subscription<std::deque<bybit::InstrumentInfo>>> mListSub;
    std::string mSymbol;
};

} // namespace scratcher::cockpit
