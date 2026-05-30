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

#ifndef SCRATCHER_DATA_CURRENCY_HPP
#define SCRATCHER_DATA_CURRENCY_HPP

#include <concepts>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace scratcher {

namespace {
    size_t parse_presision_decimals(std::string_view s)
    {
        if (s.empty()) throw std::invalid_argument("empty currency string");
        //auto s = str;
        if (s.front() == '-') s.remove_prefix(1);
        if (s.empty()) throw std::invalid_argument("empty currency string");
        if (auto point_pos = s.find('.'); point_pos != std::string_view::npos) {
            if (point_pos == s.length() - 1 || s.find('.', point_pos + 1) != std::string_view::npos) throw std::invalid_argument("invalid currency format: " + std::string(s));
            return s.length() - point_pos - 1;
        }
        return 0;
    }
}

template<std::integral T/*, std::derived_from<fixed_point_spec> SPEC*/>
class currency
{
    static constexpr T MAX_PARSE = std::numeric_limits<T>::max()/10;
    size_t m_decimals;
    T m_multiplier;
    T m_value;

public:
    constexpr currency() : m_decimals(0), m_multiplier(1), m_value(0) {}

    template <std::integral I>
    constexpr currency(I val, size_t decimals)
        : m_decimals(decimals), m_multiplier(std::pow(10, decimals)), m_value(static_cast<T>(val)) {}

    explicit currency(std::string_view str) : currency(0, parse_presision_decimals(str))
    { parse(str); }

    explicit currency(std::string_view str, size_t decimals) : currency(0, decimals)
    { parse(str); }

    currency(const currency& c) = default;

    currency& operator=(const currency& c) = default;

    currency operator-() const { return currency(-m_value, m_decimals); }

    currency& negate() { m_value = -m_value; return *this; }

    template <std::integral I>
    void set_raw(I raw)
    { m_value = static_cast<T>(raw); }

    bool operator==(const currency& c) const
    {
        if (m_multiplier == c.m_multiplier)
            return m_value == c.m_value;
        if (m_decimals < c.m_decimals)
            return (m_value * std::pow(10, c.m_decimals - m_decimals)) == c.m_value;

        return m_value == (c.m_value * std::pow(10, m_decimals - c.m_decimals));
    }

    bool operator!=(const currency& c) const
    { return !operator==(c); }

    bool operator<(const currency& c) const
    {
        if (m_multiplier == c.m_multiplier)
            return m_value < c.m_value;
        if (m_decimals < c.m_decimals)
            return (m_value * std::pow(10, c.m_decimals - m_decimals)) < c.m_value;

        return m_value < (c.m_value * std::pow(10, m_decimals - c.m_decimals));
    }

    const T& raw() const
    { return m_value; }

    size_t decimals() const
    { return m_decimals; }

    const T& multiplier() const
    { return m_multiplier; }

    // Raw value rescaled to a fixed number of fractional decimals (truncating toward
    // zero on narrowing). Lets a consumer normalise wire values that were each parsed
    // with their own scale onto one instrument-wide scale without re-parsing strings.
    T raw_at(size_t decimals) const
    {
        if (decimals == m_decimals) return m_value;
        T factor = 1;
        if (decimals > m_decimals) {
            for (size_t i = m_decimals; i < decimals; ++i) factor *= 10;
            return static_cast<T>(m_value * factor);
        }
        for (size_t i = decimals; i < m_decimals; ++i) factor *= 10;
        return static_cast<T>(m_value / factor);
    }

    std::string to_string() const
    {
        bool negative = m_value < 0;
        T abs_val = negative ? -m_value : m_value;
        std::string res = std::to_string(abs_val);
        if (m_decimals > 0) {
            while (res.length() < m_decimals + 1) res.insert(res.begin(), '0');
            res.insert(res.end() - m_decimals, '.');
        }
        if (negative) res.insert(res.begin(), '-');
        return res;
    }

    currency& parse(std::string_view str)
    {
        static const std::string delims = ".,' ";
        bool is_decimal = false;
        bool negative = false;
        size_t decimals = 0;
        T value = 0;

        auto it = str.begin();
        if (it != str.end() && *it == '-') { negative = true; ++it; }

        for (; it != str.end(); ++it) {
            auto c = *it;
            if (delims.find(c) != std::string::npos) {
                if (is_decimal) throw std::invalid_argument(std::string(str));
                if (c == '.') is_decimal = true;
                continue;
            }
            if (decimals > m_decimals && c != '0') throw std::overflow_error("decimals length: " + std::string(str));
            if (value > MAX_PARSE) throw std::overflow_error(std::string(str));

            value *= 10;
            if (is_decimal) ++decimals;

            if (c >= '1' && c <= '9') {
                T step = c - '1' + 1;
                if (std::numeric_limits<T>::max() - step < value) throw std::overflow_error(std::string(str));
                value += step;
            }
            else if (c != '0') throw std::invalid_argument(std::string(str));
        }

        if (decimals < m_decimals)
            value *= std::pow(10, m_decimals - decimals);
        else if (decimals > m_decimals)
            value /= std::pow(10, decimals - m_decimals);

        m_value = negative ? -value : value;
        return *this;
    }

};

// Trait / concept identifying a currency<T> specialisation. Used by generic code
// (e.g. the datahub DAO) to treat fixed-point fields uniformly without naming the
// integer storage type.
template<class> inline constexpr bool is_currency_v = false;
template<std::integral T> inline constexpr bool is_currency_v<currency<T>> = true;

template<class T> concept currency_like = is_currency_v<std::remove_cvref_t<T>>;

}

namespace std {

template<std::integral T>
std::string to_string(const scratcher::currency<T>& c)
{ return c.to_string(); }

}

#include <glaze/glaze.hpp>

// Glaze codec for any currency<T>: read/write as a JSON quoted decimal string.
// Exchange wire formats carry every fractional value as a quoted string, so a
// currency-typed entity field deserialises directly from "0.01" and serialises
// back via to_string(). An empty string ("" — sent by ByBit for unset optional
// price fields such as triggerPrice) maps to a zero currency rather than throwing,
// so a single "" field never fails the whole record's parse.
template<std::integral T>
struct glz::meta<scratcher::currency<T>> {
    static constexpr auto value = custom<
        [](scratcher::currency<T>& c, const std::string& s) {
            c = s.empty() ? scratcher::currency<T>{} : scratcher::currency<T>(s);
        },
        [](const scratcher::currency<T>& c) -> std::string { return c.to_string(); }
    >;
};

#endif // SCRATCHER_DATA_CURRENCY_HPP
