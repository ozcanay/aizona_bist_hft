#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

#include <cstddef>

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

static inline constexpr int REPLACE_ORDER_MSG_ORDER_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int REPLACE_ORDER_MSG_ORDERBOOK_ID_OFFSET = REPLACE_ORDER_MSG_ORDER_ID_OFFSET + sizeof(algocor::OrderId);
static inline constexpr int REPLACE_ORDER_MSG_SIDE_OFFSET = REPLACE_ORDER_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int REPLACE_ORDER_MSG_ORDERBOOK_POS_OFFSET = REPLACE_ORDER_MSG_SIDE_OFFSET + sizeof(algocor::Side);
static inline constexpr int REPLACE_ORDER_MSG_QUANTITY_OFFSET = REPLACE_ORDER_MSG_ORDERBOOK_POS_OFFSET + sizeof(algocor::OrderbookPosition);
static inline constexpr int REPLACE_ORDER_MSG_PRICE_OFFSET = REPLACE_ORDER_MSG_QUANTITY_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int REPLACE_ORDER_MSG_ORDER_ATTR_OFFSET = REPLACE_ORDER_MSG_PRICE_OFFSET + sizeof(algocor::Price);

static inline constexpr size_t REPLACE_ORDER_MSG_SIZE = REPLACE_ORDER_MSG_ORDER_ATTR_OFFSET + sizeof(algocor::OrderAttributes);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(REPLACE_ORDER_MSG_ORDER_ID_OFFSET == 5);
static_assert(REPLACE_ORDER_MSG_ORDERBOOK_ID_OFFSET == 13);
static_assert(REPLACE_ORDER_MSG_SIDE_OFFSET == 17);
static_assert(REPLACE_ORDER_MSG_ORDERBOOK_POS_OFFSET == 18);
static_assert(REPLACE_ORDER_MSG_QUANTITY_OFFSET == 22);
static_assert(REPLACE_ORDER_MSG_PRICE_OFFSET == 30);
static_assert(REPLACE_ORDER_MSG_ORDER_ATTR_OFFSET == 34);
static_assert(REPLACE_ORDER_MSG_SIZE == 36);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace constants

struct __attribute__((packed)) OrderReplace {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderId order_id;
    algocor::OrderbookId orderbook_id;
    algocor::Side side;
    algocor::OrderbookPosition orderbook_position;
    algocor::Quantity quantity;
    algocor::Price price;
    algocor::OrderAttributes order_attributes;

    friend std::ostream& operator<<(std::ostream& os, OrderReplace const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10) << be32toh(order.nanoseconds)
           << ", order_id: " << be64toh(order.order_id) << ", orderbook_id: " << std::setw(10) << be32toh(order.orderbook_id)
           << ", side: " << magic_enum::enum_name(order.side) << ", orderbook_position: " << std::setw(3)
           << be32toh(order.orderbook_position) << ", quantity: " << std::setw(6) << be64toh(order.quantity) << ", price: " << std::setw(6)
           << be32toh(order.price) << ", order_attributes: " << be16toh(order.order_attributes) << " }\n";
        return os;
    }
};
static_assert(constants::REPLACE_ORDER_MSG_SIZE == sizeof(OrderReplace));
static_assert(std::is_trivial_v<OrderReplace> && std::is_standard_layout_v<OrderReplace>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderReplace> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderReplace> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderReplace> {};
