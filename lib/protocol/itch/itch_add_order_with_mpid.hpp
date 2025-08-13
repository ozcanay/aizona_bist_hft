#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

#include <array>
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

static inline constexpr int ADD_ORDER_WITH_MPID_MSG_ORDER_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_ORDERBOOK_ID_OFFSET
    = ADD_ORDER_WITH_MPID_MSG_ORDER_ID_OFFSET + sizeof(algocor::OrderId);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_SIDE_OFFSET
    = ADD_ORDER_WITH_MPID_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_ORDERBOOK_POS_OFFSET = ADD_ORDER_WITH_MPID_MSG_SIDE_OFFSET + sizeof(algocor::Side);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_QUANTITY_OFFSET
    = ADD_ORDER_WITH_MPID_MSG_ORDERBOOK_POS_OFFSET + sizeof(algocor::OrderbookPosition);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_PRICE_OFFSET = ADD_ORDER_WITH_MPID_MSG_QUANTITY_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_ORDER_ATTR_OFFSET = ADD_ORDER_WITH_MPID_MSG_PRICE_OFFSET + sizeof(algocor::Price);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_LOT_TYPE_OFFSET
    = ADD_ORDER_WITH_MPID_MSG_ORDER_ATTR_OFFSET + sizeof(algocor::OrderAttributes);
static inline constexpr int ADD_ORDER_WITH_MPID_MSG_PARTICIPANT_ID_OFFSET
    = ADD_ORDER_WITH_MPID_MSG_LOT_TYPE_OFFSET + sizeof(algocor::LotType);

static inline constexpr int PARTICIPANT_ID_SIZE = 7;
static inline constexpr size_t ADD_ORDER_WITH_MPID_MSG_SIZE = ADD_ORDER_WITH_MPID_MSG_PARTICIPANT_ID_OFFSET + PARTICIPANT_ID_SIZE;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ADD_ORDER_WITH_MPID_MSG_ORDER_ID_OFFSET == 5);
static_assert(ADD_ORDER_WITH_MPID_MSG_ORDERBOOK_ID_OFFSET == 13);
static_assert(ADD_ORDER_WITH_MPID_MSG_SIDE_OFFSET == 17);
static_assert(ADD_ORDER_WITH_MPID_MSG_ORDERBOOK_POS_OFFSET == 18);
static_assert(ADD_ORDER_WITH_MPID_MSG_QUANTITY_OFFSET == 22);
static_assert(ADD_ORDER_WITH_MPID_MSG_PRICE_OFFSET == 30);
static_assert(ADD_ORDER_WITH_MPID_MSG_ORDER_ATTR_OFFSET == 34);
static_assert(ADD_ORDER_WITH_MPID_MSG_LOT_TYPE_OFFSET == 36);
static_assert(ADD_ORDER_WITH_MPID_MSG_PARTICIPANT_ID_OFFSET == 37);
static_assert(ADD_ORDER_WITH_MPID_MSG_SIZE == 44);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace constants

struct __attribute__((packed)) AddOrderWithMPID {
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
    std::array<char, constants::PARTICIPANT_ID_SIZE> participant_id;

    friend std::ostream& operator<<(std::ostream& os, AddOrderWithMPID const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10) << be32toh(order.nanoseconds)
           << ", order_id: " << std::setw(10) << be64toh(order.order_id) << ", orderbook_id: " << std::setw(10)
           << be32toh(order.orderbook_id) << ", side: " << magic_enum::enum_name(order.side) << ", orderbook_position: " << std::setw(3)
           << be32toh(order.orderbook_position) << ", quantity: " << std::setw(6) << be64toh(order.quantity) << ", price: " << std::setw(6)
           << be32toh(order.price) << ", order_attributes: " << be16toh(order.order_attributes)
           << ", lot_type: " << magic_enum::enum_name(order.lot_type) << ", participant_id: \""
           << std::string(order.participant_id.data(), order.participant_id.size()) << "\""
           << " }\n";
        return os;
    }
};
static_assert(constants::ADD_ORDER_WITH_MPID_MSG_SIZE == sizeof(AddOrderWithMPID));
static_assert(std::is_trivial_v<AddOrderWithMPID> && std::is_standard_layout_v<AddOrderWithMPID>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::AddOrderWithMPID> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::AddOrderWithMPID>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::AddOrderWithMPID> {};
