#pragma once

#include "../../types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

#include "ouch_types.hpp"

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) OrderCanceled {
    MessageType type;
    TimestampOuchNanoseconds timestamp;
    OrderToken order_token;
    OrderbookId orderbook_id;
    Side side;
    OrderId order_id;
    CancelReason cancel_reason;

    friend std::ostream& operator<<(std::ostream& os, OrderCanceled const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", timestamp: " << order.timestamp << ", order_token: \""
           << std::string(order.order_token.data(), order.order_token.size()) << "\""
           << ", orderbook_id: " << order.orderbook_id << ", side: " << magic_enum::enum_name(order.side)
           << ", order_id: " << order.order_id << ", cancel_reason: " << magic_enum::enum_name(order.cancel_reason) << " }\n";
        return os;
    }
};
static_assert(sizeof(OrderCanceled) == 37);
static_assert(std::is_trivial_v<OrderCanceled> && std::is_standard_layout_v<OrderCanceled>);
}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::OrderCanceled> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::OrderCanceled> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::OrderCanceled> {};
