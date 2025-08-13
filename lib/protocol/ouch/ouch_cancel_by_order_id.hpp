#pragma once

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) CancelOrderByOrderId {
    MessageType type;          // "Y" mapping
    OrderbookId orderbook_id;  // Order book identifier
    Side side;                 // Type of order, "B" = buy order and "S" = sell order
    OrderId order_id;          // The identifier assigned to the order by the system

    friend std::ostream& operator<<(std::ostream& os, CancelOrderByOrderId const& order)
    {
        os << "{"
           << "  type: \"" << magic_enum::enum_name(order.type) << "\""
           << ", order_book_id: " << be32toh(order.orderbook_id) << ", side: \"" << static_cast<char>(order.side) << "\""
           << ", order_id: " << be64toh(order.order_id) << " }\n";
        return os;
    }
};
static_assert(sizeof(CancelOrderByOrderId) == 14);
static_assert(std::is_trivial_v<CancelOrderByOrderId> && std::is_standard_layout_v<CancelOrderByOrderId>);

}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::CancelOrderByOrderId> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::CancelOrderByOrderId>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::CancelOrderByOrderId> {};
