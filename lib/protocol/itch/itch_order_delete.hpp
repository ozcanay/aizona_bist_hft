#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

#include <cstddef>
#include <endian.h>
#include <fmt/base.h>
#include <magic_enum.hpp>

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <iomanip>

namespace algocor::protocol::itch
{

namespace constants
{

static inline constexpr int ORDER_DELETE_MSG_ORDER_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ORDER_DELETE_MSG_ORDERBOOK_ID_OFFSET = ORDER_DELETE_MSG_ORDER_ID_OFFSET + sizeof(algocor::OrderId);
static inline constexpr int ORDER_DELETE_MSG_SIDE_OFFSET = ORDER_DELETE_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);

static inline constexpr size_t ORDER_DELETE_MSG_SIZE = ORDER_DELETE_MSG_SIDE_OFFSET + sizeof(algocor::Side);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ORDER_DELETE_MSG_ORDER_ID_OFFSET == 5);
static_assert(ORDER_DELETE_MSG_ORDERBOOK_ID_OFFSET == 13);
static_assert(ORDER_DELETE_MSG_SIDE_OFFSET == 17);
static_assert(ORDER_DELETE_MSG_SIZE == 18);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) OrderDelete {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderId order_id;
    algocor::OrderbookId orderbook_id;
    algocor::Side side;

    friend std::ostream& operator<<(std::ostream& os, OrderDelete const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10) << be32toh(order.nanoseconds)
           << ", order_id: " << be64toh(order.order_id) << ", orderbook_id: " << std::setw(10) << be32toh(order.orderbook_id)
           << ", side: " << magic_enum::enum_name(order.side) << " }\n";
        return os;
    }
};
static_assert(constants::ORDER_DELETE_MSG_SIZE == sizeof(OrderDelete));
static_assert(std::is_trivial_v<OrderDelete> && std::is_standard_layout_v<OrderDelete>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderDelete> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderDelete> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderDelete> {};
