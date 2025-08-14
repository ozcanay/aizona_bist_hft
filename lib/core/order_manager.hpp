#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <immintrin.h>  // For AVX2
#include <stdexcept>
#include <string>

#include <iostream>

#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

#include "../utility/overwrite_macros.hpp"

namespace
{

[[nodiscard]] std::string toString(const std::array<char, 14>& token)
{
    std::string str;
    str.resize(token.size());
    for (int i = 0; i < token.size(); ++i) {
        str[i] = token[i];
    }
    return str;
}

void printToken(const std::array<char, 14>& token)
{
    LOG_TRACE_L1("Token: {}", toString(token));
}

}  // namespace

namespace algocor
{

// this might need approximation
static constexpr size_t ORDERS_SIZE = 1 << 20;  // 1'048'576

// TODO: is this the ideal structure. do I need to pack it, do I need to rearrange fields?
struct Order {
    uint64_t quantity;
    uint32_t orderbook_id;
    int32_t price;
    char side;
    bool is_valid {
        true
    };  // if fully executed or canceled or rejected, this will be false. -> TODO: I AM NOT SURE ABOUT THE USE CASE OF THIS.
    // 3 bytes of padding added to maintain alignment
};
static_assert(sizeof(Order) == 24);

// this is not supposed to thread-safe. each OUCH session will have its own order manager.
// this class will only be modified upon receiving responses (not requests!)
class OrderManager {
public:
    explicit OrderManager(uint32_t last_order_token_index = 0)
    {
        int index = last_order_token_index + 1;
        for (int i = 13; i >= 0; --i) {
            m_token[i] = '0' + (index % 10);
            index /= 10;
        }

        printToken(m_token);
    }

    ~OrderManager()
    {
        uint32_t token_index = 0;
        for (char c : m_token) {
            token_index = token_index * 10 + (c - '0');
        }

        nlohmann::json j;
        j["last_used_order_token_index"] = token_index;

        std::ofstream file("app_parameters.json");
        if (file.is_open()) {
            file << j.dump(4);
        } else {
            std::cerr << "Failed to save last_used_order_token_index to app_parameters.json" << std::endl;
        }
    }

    void orderAccepted(const std::array<char, 14>& token, uint64_t quantity, uint32_t orderbook_id, int32_t price, char side)
    {
        const auto index = parse_order_token_decimal(token);

        if (!withinRange(index)) [[unlikely]] {
            return;
        }

        LOG_TRACE_L3("Index: {}", index);

        auto& order = m_orders[index];
        order.orderbook_id = orderbook_id;
        order.quantity = quantity;
        order.price = price;
        order.side = side;

        LOG_TRACE_L3("Token: {}. Order accepted to order manager. Index: {}. Orderbook ID: {}, side: {}, qty: {}, price: {}",
            toString(token),
            index,
            be32toh(order.orderbook_id),
            side,
            quantity,
            price);
    }

    // we will only support price modification.
    void orderReplaced(const std::array<char, 14>& replacement_token, int32_t price)
    {
        std::array<char, 14> original_order_token {};
        for (size_t i = m_originalToReplacementTokens.size() - 1; i >= 0; --i) {
            if (m_originalToReplacementTokens[i].second == replacement_token) {
                original_order_token = m_originalToReplacementTokens[i].first;
                break;
            }
        }

        if (original_order_token.empty()) [[unlikely]] {
            LOG_ERROR("Original order token not found for the replaced order. Replacement  token: {}", toString(replacement_token));
            return;
        }

        LOG_TRACE_L3("Original order token {} found for replacement token {} on order replaced",
            toString(original_order_token),
            toString(replacement_token));

        auto* order = getOrder(original_order_token);
        if (order == nullptr) [[unlikely]] {
            return;
        }

        LOG_TRACE_L3("Original Token: {}, Replacement Token: {}. Replacing order. Price {} -> {}",
            toString(original_order_token),
            toString(replacement_token),
            order->price,
            price);

        order->price = price;

        // TODO: I can also modify qty by analyzing pretrade-qty / qty.
    }

    void pendingReplace(const std::array<char, 14>& original_order_token, const std::array<char, 14>& replacement_order_token)
    {
        m_originalToReplacementTokens.emplace_back(original_order_token, replacement_order_token);
    }

    void orderExecuted(const std::array<char, 14>& token, uint64_t traded_qty)
    {
        auto* order = getOrder(token);
        if (order == nullptr) [[unlikely]] {
            return;
        }

        order->quantity -= traded_qty;
        LOG_TRACE_L3("Token: {}. Quantity after executon: {}, traded qty: {}", toString(token), order->quantity, traded_qty);

        if (order->quantity == 0) {
            LOG_TRACE_L3("Token: {}. Quantity is 0 after execution, the order is no longer active", toString(token));
            order->is_valid = false;
        }
    }

    // will be called upon canceling or rejection.
    void orderDeleted(const std::array<char, 14>& token)
    {
        auto* order = getOrder(token);
        if (order == nullptr) [[unlikely]] {
            return;
        }

        order->is_valid = false;
    }

    [[nodiscard]] std::array<char, 14> nextToken()
    {
        // for (int i = 13; i >= 0; --i) {
        //     if (m_token[i] < '9') {
        //         m_token[i]++;
        //         return m_token;
        //     } else {
        //         m_token[i] = '0';
        //     }
        // }

        // // If we get here, m_token was already at "99999999999999" (max value)
        // // throw std::overflow_error("Order token exceeded maximum value");
        // return {};

        // BRANCHLESS VERSION.
        // int carry = 1;  // Start with incrementing the least significant digit
        // for (int i = 13; i >= 0 && carry; --i) {
        //     carry = (m_token[i] == '9');  // Check if a carry is needed
        //     m_token[i] = carry ? '0' : (m_token[i] + 1);
        // }

        // // ++m_index;

        // return m_token;

        // branchless. CMOV instruction.
        // https://yarchive.net/comp/linux/cmov.html
        // https://stackoverflow.com/questions/74552752/in-assembly-should-branchless-code-use-complementary-cmovs
        int carry = 1;  // Start with incrementing the least significant digit
        for (int i = 13; i >= 0; --i) {
            int is_nine = (m_token[i] == '9');  // 1 if '9', 0 otherwise
            m_token[i] = '0' + ((m_token[i] - '0' + carry) % 10);
            carry = is_nine;  // Carry propagates only if it was '9'
        }

        return m_token;
    }

private:
    alignas(64) std::array<Order, ORDERS_SIZE> m_orders {};
    std::array<char, 14> m_token {};
    // size_t m_index = 0;

    // TODO: maybe use std::array here.
    std::vector<std::pair<std::array<char, 14>, std::array<char, 14>>> m_originalToReplacementTokens;

    // TODO: force inline this?
    [[nodiscard]] Order* getOrder(const std::array<char, 14>& token)
    {
        const auto index = parse_order_token_decimal(token);
        if (!withinRange(index)) [[unlikely]] {
            return nullptr;
        }

        return &m_orders[index];
    }

    // TODO: in future, we may prefer to store numbers in hexadecimal format, instead of decimal formatting.
    [[nodiscard]] size_t parse_order_token_decimal(const std::array<char, 14>& token)
    {
        size_t index = 0;

        // Parse the decimal number manually (avoids allocations)
        for (char c : token) {
            // if (c < '0' || c > '9') {
            //     throw std::invalid_argument("Invalid decimal order token");
            // }
            index = index * 10 + (c - '0');
        }

        return index;

        // SIMD version:

        // __m128i chars = _mm_loadu_si128(reinterpret_cast<const __m128i*>(token.data()));

        // // Convert ASCII to integer values
        // __m128i ascii_offset = _mm_set1_epi8('0');
        // __m128i digits = _mm_sub_epi8(chars, ascii_offset);

        // // Multiply by powers of 10 and sum up. should I make this static?
        // const int64_t powers_of_10[14] = { 1,
        //     10,
        //     100,
        //     1000,
        //     10000,
        //     100000,
        //     1000000,
        //     10000000,
        //     100000000,
        //     1000000000,
        //     10000000000,
        //     100000000000,
        //     1000000000000,
        //     10000000000000 };

        // size_t result = 0;
        // for (int i = 0; i < 14; ++i) {
        //     result += static_cast<size_t>(token[i] - '0') * powers_of_10[13 - i];
        // }

        // return result;
    }

    // TODO: maybe forceinline this.
    [[nodiscard]] bool withinRange(size_t index)
    {
        if (index >= m_orders.size()) [[unlikely]] {
            LOG_ERROR("Index is larger than order array size. Not adding order to the order manager");
            return false;
        }

        return true;
    }
};

}  // namespace algocor
