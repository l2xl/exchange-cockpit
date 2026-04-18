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

#include <memory>
#include <functional>
#include <string>
#include <vector>

#include <elements.hpp>

#include "instrument_panel.hpp"
#include "ui_builder.hpp"

namespace scratcher::elements {

class ScratchPanel;

class ElementsInstrumentPanel : public cockpit::InstrumentPanel
{
    struct EnsurePrivate {};

public:
    ElementsInstrumentPanel(cockpit::PanelType type, std::weak_ptr<cycfi::elements::view> view, std::weak_ptr<IDataController> controller, InstrumentPanelWidgets widgets, EnsurePrivate);

    static std::shared_ptr<ElementsInstrumentPanel> Create(cockpit::PanelType type, std::weak_ptr<cycfi::elements::view> view, std::weak_ptr<IDataController> controller, InstrumentPanelWidgets widgets);

    void SetDataReady(bool ready) override;
    void Update() override;

protected:
    void PostToUi(std::function<void()> fn) override;
    void OnInstrumentsReady(std::vector<std::string> symbols) override;
    void OnSymbolSelected(const std::string& symbol) override;

private:
    std::weak_ptr<cycfi::elements::view> mView;
    InstrumentPanelWidgets mWidgets;
    std::weak_ptr<ScratchPanel> mScratchPanel;
};

} // namespace scratcher::elements
