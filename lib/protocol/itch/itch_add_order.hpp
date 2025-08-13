#pragma once

#include <cstddef>

#include "../../constants.hpp"
#include "../../types.hpp"
#include <fmt/base.h>
#include <iomanip>
#include <magic_enum.hpp>
#include <quill/bundled/fmt/ostream.h>
#include <quill/core/Codec.h>
#include <quill/TriviallyCopyableCodec.h>

namespace algocor::protocol::itch
{

namespace constants
{

static inline constexpr int ADD_ORDER_MSG_ORDER_ID_OFFSET = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ADD_ORDER_MSG_ORDERBOOK_ID_OFFSET = ADD_ORDER_MSG_ORDER_ID_OFFSET + sizeof(algocor::OrderId);
static inline constexpr int ADD_ORDER_MSG_SIDE_OFFSET = ADD_ORDER_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int ADD_ORDER_MSG_ORDERBOOK_POS_OFFSET = ADD_ORDER_MSG_SIDE_OFFSET + sizeof(algocor::Side);
static inline constexpr int ADD_ORDER_MSG_QUANTITY_OFFSET = ADD_ORDER_MSG_ORDERBOOK_POS_OFFSET + sizeof(algocor::OrderbookPosition);
static inline constexpr int ADD_ORDER_MSG_PRICE_OFFSET = ADD_ORDER_MSG_QUANTITY_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int ADD_ORDER_MSG_ORDER_ATTR_OFFSET = ADD_ORDER_MSG_PRICE_OFFSET + sizeof(algocor::Price);
static inline constexpr int ADD_ORDER_MSG_LOT_TYPE_OFFSET = ADD_ORDER_MSG_ORDER_ATTR_OFFSET + sizeof(algocor::OrderAttributes);

static inline constexpr size_t ADD_ORDER_MSG_SIZE = ADD_ORDER_MSG_LOT_TYPE_OFFSET + sizeof(algocor::LotType);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ADD_ORDER_MSG_ORDER_ID_OFFSET == 5);
static_assert(ADD_ORDER_MSG_ORDERBOOK_ID_OFFSET == 13);
static_assert(ADD_ORDER_MSG_SIDE_OFFSET == 17);
static_assert(ADD_ORDER_MSG_ORDERBOOK_POS_OFFSET == 18);
static_assert(ADD_ORDER_MSG_QUANTITY_OFFSET == 22);
static_assert(ADD_ORDER_MSG_PRICE_OFFSET == 30);
static_assert(ADD_ORDER_MSG_ORDER_ATTR_OFFSET == 34);
static_assert(ADD_ORDER_MSG_LOT_TYPE_OFFSET == 36);
static_assert(ADD_ORDER_MSG_SIZE == 37);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace constants

struct __attribute__((packed)) AddOrder {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderId order_id;
    algocor::OrderbookId orderbook_id;
    algocor::Side side;
    algocor::OrderbookPosition orderbook_position;
    algocor::Quantity quantity;
    algocor::Price price;
    algocor::OrderAttributes order_attributes;
    algocor::LotType lot_type;

    friend std::ostream& operator<<(std::ostream& os, AddOrder const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10)
           << be32toh(order.nanoseconds)  // TODO: use std::byteswap or something similar.
           << ", order_id: " << be64toh(order.order_id) << ", orderbook_id: " << std::setw(10) << be32toh(order.orderbook_id)
           << ", side: " << magic_enum::enum_name(order.side) << ", orderbook_position: " << std::setw(3)
           << be32toh(order.orderbook_position) << ", quantity: " << std::setw(6) << be64toh(order.quantity) << ", price: " << std::setw(6)
           << be32toh(order.price) << ", order_attributes: " << be16toh(order.order_attributes)
           << ", lot_type: " << magic_enum::enum_name(order.lot_type) << " }\n";
        return os;
    }
};
static_assert(constants::ADD_ORDER_MSG_SIZE == sizeof(AddOrder));
static_assert(std::is_trivial_v<AddOrder> && std::is_standard_layout_v<AddOrder>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::AddOrder> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::AddOrder> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::AddOrder> {};
