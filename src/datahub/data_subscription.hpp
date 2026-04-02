// Scratcher project
// Copyright (c) 2025-2026 l2xl (l2xl/at/proton.me)
// Distributed under the Intellectual Property Reserve License (IPRL)
// -----BEGIN PGP PUBLIC KEY BLOCK-----
// mDMEYdxcVRYJKwYBBAHaRw8BAQdAfacBVThCP5QDPEgSbSIudtpJS4Y4Imm5dzaN
// lM1HTem0IkwyIFhsIChsMnhsKSA8bDJ4bEBwcm90b21tYWlsLmNvbT6IkAQTFggA
// OBYhBKRCfUyWnduCkisNl+WRcOaCK79JBQJh3FxVAhsDBQsJCAcCBhUKCQgLAgQW
// AgMBAh4BAheAAAoJEOWRcOaCK79JDl8A/0/AjYVbAURZJXP3tHRgZyYyN9txT6mW
// 0bYCcOf0rZ4NAQDoFX4dytPDvcjV7ovSQJ6dzvIoaRbKWGbHRCufrm5QBA==
// =KKu7
// -----END PGP PUBLIC KEY BLOCK-----

#ifndef DATAHUB_DATA_SUBSCRIPTION_HPP
#define DATAHUB_DATA_SUBSCRIPTION_HPP

#include <memory>
#include <type_traits>
#include <utility>
#include <ranges>

#include "data_update.hpp"
#include "generic_handler.hpp"

namespace datahub {

using scratcher::generic_handler;

template<std::ranges::input_range Range>
class data_subscription : public std::enable_shared_from_this<data_subscription<Range>>
{
    struct ensure_private {};
public:
    using range_view_type = std::ranges::subrange<std::ranges::iterator_t<const Range>>;
    using data_type = std::pair<update_kind, range_view_type>;

    explicit data_subscription(ensure_private) {}
    virtual ~data_subscription() = default;
    virtual void handle_data(data_type&& data) = 0;

    template<typename Callable>
    static std::shared_ptr<data_subscription> create(Callable&& handler)
    {
        return std::make_shared<generic_handler<data_type, data_subscription, Callable, void, ensure_private>>(std::forward<Callable>(handler), ensure_private{});
    }

};

template<std::ranges::input_range Range, typename Callable>
auto make_data_subscription(Callable&& handler)
{
    return data_subscription<Range>::create(std::forward<Callable>(handler));
}

} // namespace datahub

#endif // DATAHUB_DATA_SUBSCRIPTION_HPP
