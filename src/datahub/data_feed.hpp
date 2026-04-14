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

#ifndef DATAHUB_DATA_FEED_HPP
#define DATAHUB_DATA_FEED_HPP

#include <memory>
#include <functional>
#include <deque>
#include <list>
#include <algorithm>
#include <concepts>
#include <ranges>
#include "data_subscription.hpp"
#include "data_condition.hpp"

namespace datahub {

template<typename Entity, auto SortField, auto KeyField>
class sorted_data_feed : public std::enable_shared_from_this<sorted_data_feed<Entity, SortField, KeyField>>
{
public:
    using entity_type = Entity;
    using cache_type = std::deque<entity_type>;
    using condition_type = data_condition<entity_type>;
    using subscription_type = data_subscription<cache_type>;
private:
    cache_type m_cache;
    std::list<std::weak_ptr<subscription_type>> m_subscriptions;
    std::shared_ptr<condition_type> m_condition;

    template<std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, entity_type>
    void push_to_subscriptions(Range&& data, update_kind kind)
    {
        typename subscription_type::range_view_type view(std::ranges::begin(data), std::ranges::end(data));
        auto it = m_subscriptions.begin();
        while (it != m_subscriptions.end()) {
            if (auto sub = it->lock()) {
                sub->handle_data(typename subscription_type::data_type(kind, view));
                ++it;
            }
            else {
                it = m_subscriptions.erase(it);
            }
        }
    }

public:
    explicit sorted_data_feed(std::shared_ptr<condition_type> condition = {})
        : m_condition(std::move(condition))
    {}

    static std::shared_ptr<sorted_data_feed> create(std::shared_ptr<condition_type> condition = {})
    { return std::make_shared<sorted_data_feed>(std::move(condition)); }

    void set_condition(std::shared_ptr<condition_type> condition) { m_condition = std::move(condition); }

    void subscribe(std::weak_ptr<subscription_type> sub)
    {
        if (!m_cache.empty())
            if (auto locked = sub.lock())
                locked->handle_data(typename subscription_type::data_type(update_kind::snapshot, typename subscription_type::range_view_type(m_cache.cbegin(), m_cache.cend())));
        m_subscriptions.emplace_back(std::move(sub));
    }

    const cache_type& get_snapshot() const { return m_cache; }

    template<std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, entity_type>
    auto data_acceptor()
    {
        static auto sort_val = [](const entity_type& e) -> decltype(auto) { return e.*SortField; };
        static auto key_val = [](const entity_type& e) -> decltype(auto) { return e.*KeyField; };
        std::weak_ptr<sorted_data_feed> ref = this->weak_from_this();
        return [ref](Range&& entities) {
            if (auto self = ref.lock()) {

                // Phase 1: Filter incoming by condition
                auto incoming = std::forward<Range>(entities)
                    | std::views::filter([&](const entity_type& e) { return !self->m_condition || self->m_condition->matches(e); })
                    /*| std::ranges::to<cache_type>()*/;
                if (incoming.empty()) return;

                size_t inserted = 0;
                update_kind update = update_kind::increment;
                auto first_update_it = self->m_cache.end();
                if (self->m_cache.empty()) {
                    self->m_cache = std::ranges::to<cache_type>(incoming);
                    update = update_kind::snapshot;
                    inserted = self->m_cache.size();
                }
                else {
                    auto cache_it = self->m_cache.begin();
                    auto in_end = std::ranges::end(incoming);
                    for (auto in_it = std::ranges::begin(incoming); in_it != in_end; ++in_it) {

                        while (cache_it != self->m_cache.end() && sort_val(*cache_it) < sort_val(*in_it))
                            ++cache_it;

                        if (cache_it == self->m_cache.end()) {
                            auto it = self->m_cache.insert(cache_it, in_it, in_end);
                            if (first_update_it == self->m_cache.end())
                                first_update_it = it;
                            ++inserted;
                            break;
                        }

                        if (sort_val(*in_it) < sort_val(*cache_it)) {
                            cache_it = self->m_cache.emplace(cache_it, std::move(*in_it));
                            update = update_kind::snapshot;
                            ++inserted;
                        } else if (sort_val(*in_it) == sort_val(*cache_it)) {
                            if (key_val(*in_it) != key_val(*cache_it)) {
                                cache_it = self->m_cache.emplace(cache_it, std::move(*in_it));
                                update = update_kind::snapshot;
                                ++inserted;
                            }
                        }
                        ++cache_it;
                    }
                }
                if (inserted > 0) {
                    if (update == update_kind::snapshot) {
                        self->push_to_subscriptions(std::ranges::subrange(self->m_cache.begin(), self->m_cache.end()), update);
                    }
                    else {
                        self->push_to_subscriptions(std::ranges::subrange(first_update_it, self->m_cache.end()), update);
                    }
                }
            }
        };
    }
};

template<typename Entity, auto SortField, auto KeyField>
class sorted_snapshot_data_feed : public std::enable_shared_from_this<sorted_snapshot_data_feed<Entity, SortField, KeyField>>
{
public:
    using entity_type = Entity;
    using cache_type = std::deque<entity_type>;
    using condition_type = data_condition<entity_type>;
    using subscription_type = data_subscription<cache_type>;

private:
    cache_type m_cache;
    std::list<std::weak_ptr<subscription_type>> m_subscriptions;
    std::shared_ptr<condition_type> m_condition;

    void push_to_subscriptions(const cache_type& data, update_kind kind)
    {
        typename subscription_type::range_view_type view(data.cbegin(), data.cend());
        auto it = m_subscriptions.begin();
        while (it != m_subscriptions.end()) {
            if (auto sub = it->lock()) { sub->handle_data(typename subscription_type::data_type(kind, view)); ++it; }
            else { it = m_subscriptions.erase(it); }
        }
    }

public:
    explicit sorted_snapshot_data_feed(std::shared_ptr<condition_type> condition = {})
        : m_condition(std::move(condition))
    {}

    static std::shared_ptr<sorted_snapshot_data_feed> create(std::shared_ptr<condition_type> condition = {})
    { return std::make_shared<sorted_snapshot_data_feed>(std::move(condition)); }

    void set_condition(std::shared_ptr<condition_type> condition) { m_condition = std::move(condition); }

    void subscribe(std::weak_ptr<subscription_type> sub)
    {
        if (!m_cache.empty())
            if (auto locked = sub.lock())
                locked->handle_data(typename subscription_type::data_type(update_kind::snapshot, typename subscription_type::range_view_type(m_cache.cbegin(), m_cache.cend())));
        m_subscriptions.emplace_back(std::move(sub));
    }

    const cache_type& get_snapshot() const { return m_cache; }

    template<std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, entity_type>
    auto data_acceptor()
    {
        static auto sort_val = [](const entity_type& e) -> decltype(auto) { return e.*SortField; };
        static auto key_val = [](const entity_type& e) -> decltype(auto) { return e.*KeyField; };
        std::weak_ptr ref = this->weak_from_this();
        return [ref](Range&& entities) {
            if (auto self = ref.lock()) {
                auto incoming = std::forward<Range>(entities)
                    | std::views::filter([&](const entity_type& e) { return !self->m_condition || self->m_condition->matches(e); });
                if (std::ranges::begin(incoming) == std::ranges::end(incoming)) return;

                size_t inserted = 0;
                if (self->m_cache.empty()) {
                    self->m_cache = std::ranges::to<cache_type>(incoming);
                    inserted = self->m_cache.size();
                }
                else {
                    auto cache_it = self->m_cache.begin();
                    auto in_end = std::ranges::end(incoming);
                    for (auto in_it = std::ranges::begin(incoming); in_it != in_end; ++in_it) {
                        while (cache_it != self->m_cache.end() && sort_val(*cache_it) < sort_val(*in_it))
                            ++cache_it;

                        if (cache_it == self->m_cache.end()) {
                            self->m_cache.insert(cache_it, in_it, in_end);
                            ++inserted;
                            break;
                        }

                        if (sort_val(*in_it) < sort_val(*cache_it)) {
                            cache_it = self->m_cache.emplace(cache_it, std::move(*in_it));
                            ++inserted;
                        } else if (sort_val(*in_it) == sort_val(*cache_it)) {
                            if (key_val(*in_it) != key_val(*cache_it)) {
                                cache_it = self->m_cache.emplace(cache_it, std::move(*in_it));
                                ++inserted;
                            }
                        }
                        ++cache_it;
                    }
                }
                if (inserted > 0)
                    self->push_to_subscriptions(self->m_cache, update_kind::snapshot);
            }
        };
    }
};

template<typename Entity, auto KeyField>
class keyed_snapshot_data_feed : public std::enable_shared_from_this<keyed_snapshot_data_feed<Entity, KeyField>>
{
public:
    using entity_type = Entity;
    using cache_type = std::deque<entity_type>;
    using condition_type = data_condition<entity_type>;
    using subscription_type = data_subscription<cache_type>;

private:
    cache_type m_cache;
    std::list<std::weak_ptr<subscription_type>> m_subscriptions;
    std::shared_ptr<condition_type> m_condition;

    void push_to_subscriptions()
    {
        typename subscription_type::range_view_type view(m_cache.cbegin(), m_cache.cend());
        auto it = m_subscriptions.begin();
        while (it != m_subscriptions.end()) {
            if (auto sub = it->lock()) { sub->handle_data(typename subscription_type::data_type(update_kind::snapshot, view)); ++it; }
            else { it = m_subscriptions.erase(it); }
        }
    }

public:
    explicit keyed_snapshot_data_feed(std::shared_ptr<condition_type> condition = {})
        : m_condition(std::move(condition))
    {}

    static std::shared_ptr<keyed_snapshot_data_feed> create(std::shared_ptr<condition_type> condition = {})
    { return std::make_shared<keyed_snapshot_data_feed>(std::move(condition)); }

    void set_condition(std::shared_ptr<condition_type> condition) { m_condition = std::move(condition); }

    void subscribe(std::weak_ptr<subscription_type> sub)
    {
        if (!m_cache.empty())
            if (auto locked = sub.lock())
                locked->handle_data(typename subscription_type::data_type(update_kind::snapshot, typename subscription_type::range_view_type(m_cache.cbegin(), m_cache.cend())));
        m_subscriptions.emplace_back(std::move(sub));
    }

    const cache_type& get_snapshot() const { return m_cache; }

    template<std::ranges::input_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, entity_type>
    auto data_acceptor()
    {
        static auto key_val = [](const entity_type& e) -> decltype(auto) { return e.*KeyField; };
        std::weak_ptr<keyed_snapshot_data_feed> ref = this->weak_from_this();
        return [ref](Range&& entities) {
            if (auto self = ref.lock()) {
                bool changed = false;
                for (auto in_it = std::ranges::begin(entities); in_it != std::ranges::end(entities); ++in_it) {
                    if (self->m_condition && !self->m_condition->matches(*in_it)) continue;
                    auto cache_it = std::ranges::find_if(self->m_cache, [&](const entity_type& e) { return key_val(e) == key_val(*in_it); });
                    if (cache_it == self->m_cache.end())
                        self->m_cache.emplace_back(std::move(*in_it));
                    else
                        *cache_it = std::move(*in_it);
                    changed = true;
                }
                if (changed)
                    self->push_to_subscriptions();
            }
        };
    }
};

} // namespace datahub

#endif // DATAHUB_DATA_FEED_HPP
