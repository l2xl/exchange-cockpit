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

#ifndef SCRATCHER_ORDERBOOK_HPP
#define SCRATCHER_ORDERBOOK_HPP

#include <vector>
#include <memory>
#include <algorithm>
#include <ranges>

#include "entities/orderbook_level.hpp"

namespace scratcher {

// In-memory orderbook model for data_provider parameterization
// Maintains sorted levels (price descending) and merges snapshot/delta updates
class OrderBook : public std::enable_shared_from_this<OrderBook>
{
public:
    using entity_type = OrderBookLevel;
    using cache_type = std::vector<entity_type>;

private:
    std::vector<OrderBookLevel> mLevels;

    struct EnsurePrivate {};

public:
    explicit OrderBook(EnsurePrivate) {}

    static std::shared_ptr<OrderBook> Create()
    { return std::make_shared<OrderBook>(EnsurePrivate{}); }

    template <std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, OrderBookLevel>
    void Merge(Range&& incoming)
    {
        for (const auto& level : incoming) {
            auto it = std::ranges::find_if(mLevels, [&](const OrderBookLevel& l) { return l.price == level.price; });
            if (level.size.raw() == 0) {
                if (it != mLevels.end()) mLevels.erase(it);
            }
            else if (it != mLevels.end()) {
                it->size = level.size;
            }
            else {
                // price-descending order: a comes before b when a.price > b.price, i.e. b.price < a.price
                auto pos = std::lower_bound(mLevels.begin(), mLevels.end(), level, [](const OrderBookLevel& a, const OrderBookLevel& b) { return b.price < a.price; });
                mLevels.insert(pos, level);
            }
        }
    }

    template <std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, OrderBookLevel>
    void Snapshot(Range&& levels)
    {
        mLevels.clear();
        std::ranges::move(std::forward<Range>(levels), std::back_inserter(mLevels));
        std::ranges::sort(mLevels, [](const OrderBookLevel& a, const OrderBookLevel& b) { return b.price < a.price; });
    }

    auto begin() const { return mLevels.begin(); }
    auto end() const { return mLevels.end(); }
    const cache_type& Levels() const { return mLevels; }

    template <std::ranges::input_range Range, typename Container = cache_type>
    requires std::convertible_to<std::ranges::range_value_t<Range>, OrderBookLevel>
    auto data_acceptor()
    {
        std::weak_ptr<OrderBook> ref = shared_from_this();
        return [ref](Range&& entities) -> Container {
            if (auto self = ref.lock()) {
                self->Merge(std::forward<Range>(entities));
                return Container(self->mLevels.begin(), self->mLevels.end());
            }
            return {};
        };
    }
};

} // namespace scratcher

#endif // SCRATCHER_ORDERBOOK_HPP
