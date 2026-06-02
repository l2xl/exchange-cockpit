// Scratcher project
// Copyright (c) 2025 l2xl (l2xl/at/proton.me)
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

#include <limits>

#include "timedef.hpp"
#include "buoy_candle.hpp"


namespace scratcher {

void BuoyCandleQuotes::AdvanceTo(uint64_t now_ts, price_t last_price)
{
    if (!m_first_buoy_timestamp)
        return;

    uint64_t active_buoy_ts = now_ts - now_ts % buoy_duration();
    uint64_t next_buoy_ts = *m_first_buoy_timestamp + m_buoy_data.size() * buoy_duration();
    while (active_buoy_ts > next_buoy_ts) {
        candle_t buoy = mCurCandle;
        reset_active(last_price);
        m_buoy_data.push_back(buoy);
        next_buoy_ts += buoy_duration();
    }
}

}
