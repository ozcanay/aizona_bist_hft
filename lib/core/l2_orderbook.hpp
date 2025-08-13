#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <utility>
#include <vector>

#include "../utility/overwrite_macros.hpp"
#include "../utility/quill_wrapper.hpp"

// TODO: align this.
/*struct LightAddOrder
{
    std::uint64_t order_id;
    std::uint32_t orderbook_id;
    char side;               // looking at price and market price, I can infer this?
    std::uint64_t quantity;  // can I fit this in 4 bytes?
    std::int32_t price;
};*/

inline void HandleError()
{
    LOG_ERROR("Error being handled.");
    // std::abort();
}

#define EXPECT(cond)                                                                                                                       \
    if (!(cond)) [[unlikely]] {                                                                                                            \
        [&]() __attribute__((noinline, cold)) { HandleError(); }();                                                                        \
    }

// Reference: When Nanoseconds Matter: Ultrafast Trading Systems in C++ - David Gross - CppCon 2024
// https://www.youtube.com/watch?v=sX2nF1fW7kI
class L2Orderbook {
private:
    std::vector<std::pair<std::int32_t, std::uint64_t>> m_bids;
    std::vector<std::pair<std::int32_t, std::uint64_t>> m_asks;

    template<typename Comparator>
    void AddOrder(std::vector<std::pair<std::int32_t, std::uint64_t>>& levels, std::int32_t price, std::uint64_t qty, Comparator comp)
    {
        if (levels.empty()) [[unlikely]] {
            levels.emplace_back(price, qty);
            LOG_DEBUG("ADDED ORDER TO EMPTY BOOK SIDE");
            return;
        }

        auto iter = GetLevel(levels, price);
        Dump();
        LOG_DEBUG("NOW ADDING ORDER");

        if (iter != levels.rend() && iter->first == price) [[likely]] {
            iter->second += qty;
        } else {
            std::vector<std::pair<std::int32_t, std::uint64_t>>::reverse_iterator level_iter;
            for (level_iter = levels.rbegin(); level_iter != levels.rend(); ++level_iter) {
                LOG_DEBUG("X: {}, Y: {}", price, level_iter->first);
                if (comp(level_iter->first, price)) {
                    LOG_DEBUG("COMP TRUE");
                    levels.insert(level_iter.base(), { price, qty });
                    break;
                } else {
                    LOG_DEBUG("COMP FALSE");
                }
            }

            if (level_iter == levels.rend()) {
                LOG_DEBUG("INSERTED TO THE WORST LEVEL!");
                levels.insert(levels.begin(), { price, qty });
            }

            // if (!levels.empty() && comp(price, levels.back().first)) {
            //     // best level.
            //     levels.emplace_back(price, qty);
            //     LOG_DEBUG("adding qty to the brand-new best level");
            // } else {
            //     // worst level.
            //     // this should occur very rarely!
            //     // TODO: this segfaults when reallocation happens, to remedy this, I reserved space beforehand.
            //     levels.insert(levels.begin(), { price, qty });
            //     LOG_DEBUG("adding qty to the brand-new worst level");
            // }
        }

        LOG_DEBUG("ADDED ORDER");
    }

    void DeleteOrder(std::vector<std::pair<std::int32_t, std::uint64_t>>& levels, std::int32_t price, std::uint64_t qty)
    {
        auto iter = GetLevel(levels, price);

        if (!(iter != levels.rend() && iter->first == price)) {
            LOG_ERROR("TO BE DELETED ORDER NOT FOUND. DUMPING ORDERBOOK. PRICE: {}", price);
            Dump();
            std::exit(42);

            return;
        }

        EXPECT(iter != levels.rend() && iter->first == price);

        iter->second -= qty;
        if (iter->second <= 0) [[unlikely]] {
            LOG_DEBUG("level erased for price: {}", price);
            auto it = iter.base();
            --it;
            levels.erase(it);
        }
    }

    template<typename Comparator>
    void ReplaceOrder(std::vector<std::pair<std::int32_t, std::uint64_t>>& levels,
        std::int32_t oldPrice,
        std::uint64_t oldQty,
        std::int32_t newPrice,
        std::uint64_t newQty,
        Comparator comp)
    {
        auto iter = GetLevel(levels, oldPrice);

        EXPECT(iter != levels.rend() && iter->first == oldPrice);

        // Case 1: oldPrice == newPrice, only adjust quantities
        if (oldPrice == newPrice) {
            EXPECT(iter->second >= oldQty);  // Ensure sufficient quantity to replace
            iter->second = iter->second - oldQty + newQty;
            return;
        }

        // Case 2: oldPrice != newPrice
        // Adjust or remove the old order
        if (iter->second > oldQty) [[likely]] {
            iter->second -= oldQty;
        } else {
            auto it = iter.base();
            --it;
            levels.erase(it);
        }

        // Insert or update the new order
        AddOrder(levels, newPrice, newQty, comp);
    }

    void ExecuteOrderImpl(std::vector<std::pair<std::int32_t, std::uint64_t>>& levels, std::int32_t price, std::uint64_t qty)
    {
        // Find the price level using branchless lower bound
        auto iter = GetLevel(levels, price);
        LOG_DEBUG("EXECUTE PRICE: {}, QTY: {}", price, qty);
        Dump();

        // Ensure the price level exists and matches the input price
        EXPECT(iter != levels.rend() && iter->first == price);

        // Decrement the quantity
        if (iter->second > qty) [[likely]] {
            iter->second -= qty;
        } else {
            // If the quantity is fully consumed, remove the price level
            auto it = iter.base();
            --it;
            levels.erase(it);
        }
    }

public:
    L2Orderbook()
    {
        m_bids.reserve(1'000'000);
        m_asks.reserve(1'000'000);
    }

    const std::vector<std::pair<std::int32_t, std::uint64_t>>::reverse_iterator
    GetLevel(std::vector<std::pair<std::int32_t, std::uint64_t>>& levels, int32_t price) const
    {
        auto iter = levels.rbegin();
        while (iter != levels.rend() && iter->first != price) {
            ++iter;
        }

        return iter;
    }

    void AddOrder(char side, std::int32_t price, std::uint64_t qty)
    {
        if (side == 'B') {
            AddOrder(m_bids, price, qty, std::less<>());  // top of the book is at the end.
        } else {
            AddOrder(m_asks, price, qty, std::greater<>());  // top of the book is at the end.
        }
    }

    void DeleteOrder(char side, std::int32_t price, std::uint64_t qty)
    {
        if (side == 'B') {
            DeleteOrder(m_bids, price, qty);  // top of the book is at the end.
        } else {
            DeleteOrder(m_asks, price, qty);  // top of the book is at the end.
        }
    }

    void ReplaceOrder(char side, std::int32_t oldPrice, std::uint64_t oldQty, std::int32_t newPrice, std::uint64_t newQty)
    {
        if (side == 'B') {
            ReplaceOrder(m_bids, oldPrice, oldQty, newPrice, newQty, std::less<>());  // top of the book is at the end.
        } else {
            ReplaceOrder(m_asks, oldPrice, oldQty, newPrice, newQty, std::greater<>());  // top of the book is at the end.
        }
    }

    void ExecuteOrder(char side, std::int32_t price, std::uint64_t qty)
    {
        if (side == 'B') {
            ExecuteOrderImpl(m_bids, price, qty);
        } else {
            ExecuteOrderImpl(m_asks, price, qty);
        }
    }

    [[nodiscard]] std::pair<std::int32_t, std::int32_t> GetBestPrices() const
    {
        return { m_bids.rbegin()->first, m_asks.rbegin()->first };
    }

    void Dump()
    {
        int32_t prev_price = INT_MAX;

        LOG_TRACE_L3("ASKS");
        bool bad = false;
        for (const auto& [ask_px, ask_qty] : m_asks) {
            if (prev_price <= ask_px) {
                bad = true;
            }
            if (ask_qty == 0) {
                bad = true;
            }

            prev_price = ask_px;
            LOG_TRACE_L3("{} - {}", ask_px, ask_qty);
        }
        LOG_TRACE_L3("\n------------------------------------------BIDS");
        for (auto iter = m_bids.rbegin(); iter != m_bids.rend(); ++iter) {
            if (prev_price <= iter->first) {
                bad = true;
            }
            if (iter->second == 0) {
                bad = true;
            }

            prev_price = iter->first;
            LOG_TRACE_L3("{} - {}", iter->first, iter->second);
        }

        if (bad) {
            LOG_ERROR("BAD ORDERBOOK");
        }
    }
};
