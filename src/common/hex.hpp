// Scratcher project
// Copyright (c) 2026 l2xl (l2xl/at/proton.me)
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

#ifndef EXSCRATCHER_HEX_HPP
#define EXSCRATCHER_HEX_HPP
#include <array>
#include <cassert>
#include <cstdint>
#include <string>

namespace scratcher {

extern std::array<std::array<char, 2>, 256> byte_to_hex;

template<unsigned N>
std::string hex(const unsigned char (&s)[N])
{
    std::string res(N * 2, '\0');

    char* it = res.data();
    for (uint8_t v : s) {
        *it = byte_to_hex[v][0];
        ++it;
        *it = byte_to_hex[v][1];
        ++it;
    }

    assert(it == res.data() + res.size());
    return res;
}

template<typename SPAN>
std::string hex(const SPAN& s)
{
    std::string res(s.size() * 2, '\0');

    char* it = res.data();
    for (uint8_t v : s) {
        *it = byte_to_hex[v][0];
        ++it;
        *it = byte_to_hex[v][1];
        ++it;
    }

    assert(it == res.data() + res.size());
    return res;
}

}

#endif //EXSCRATCHER_HEX_HPP
