#pragma once

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{

// TODO: move this to somewhere else.
[[nodiscard]] inline std::uint64_t parseUint64(const OuchMatchId& data)
{
    std::uint64_t result = 0;
    std::memcpy(&result, data.data(), sizeof(result));
    return result;
}

struct __attribute__((packed)) OrderExecuted {
    MessageType type;
    TimestampOuchNanoseconds timestamp;
    OrderToken order_token;
    OrderbookId orderbook_id;
    Quantity traded_quantity;
    Price trade_price;
    OuchMatchId match_id;
    ClientCategory client_category;

    std::array<char, 16> reserved;

    friend std::ostream& operator<<(std::ostream& os, OrderExecuted const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", timestamp: " << order.timestamp << ", order_token: \""
           << std::string(order.order_token.data(), order.order_token.size()) << "\""
           << ", orderbook_id: " << order.orderbook_id << ", traded_quantity: " << order.traded_quantity
           << ", trade_price: " << order.trade_price << ", match_id: " << parseUint64(order.match_id)
           << ", client_category: " << magic_enum::enum_name(order.client_category) << ", reserved: \""
           << std::string(order.reserved.data(), order.reserved.size()) << "\""
           << " }\n";
        return os;
    }
};
static_assert(sizeof(OrderExecuted) == 68);
static_assert(std::is_trivial_v<OrderExecuted> && std::is_standard_layout_v<OrderExecuted>);
}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::OrderExecuted> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::OrderExecuted> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::OrderExecuted> {};
