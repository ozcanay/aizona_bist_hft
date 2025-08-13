#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

#include <array>
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

static inline constexpr int ORDER_EXEC_WITH_PX_MSG_RESERVED1_SIZE = 7;
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_RESERVED2_SIZE = 7;

static inline constexpr int ORDER_EXEC_WITH_PX_MSG_ORDER_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_ORDERBOOK_ID_OFFSET = ORDER_EXEC_WITH_PX_MSG_ORDER_ID_OFFSET + sizeof(algocor::OrderId);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_SIDE_OFFSET = ORDER_EXEC_WITH_PX_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_EXEC_QTY_OFFSET = ORDER_EXEC_WITH_PX_MSG_SIDE_OFFSET + sizeof(algocor::Side);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_MATCH_ID_OFFSET = ORDER_EXEC_WITH_PX_MSG_EXEC_QTY_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_COMBO_GROUP_ID_OFFSET
    = ORDER_EXEC_WITH_PX_MSG_MATCH_ID_OFFSET + sizeof(algocor::MatchId);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_TRADE_PX_OFFSET = ORDER_EXEC_WITH_PX_MSG_COMBO_GROUP_ID_OFFSET
                                                                     + sizeof(algocor::ComboGroupId) + ORDER_EXEC_WITH_PX_MSG_RESERVED1_SIZE
                                                                     + ORDER_EXEC_WITH_PX_MSG_RESERVED2_SIZE;
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_OCCURRED_AT_CROSS_OFFSET
    = ORDER_EXEC_WITH_PX_MSG_TRADE_PX_OFFSET + sizeof(algocor::Price);
static inline constexpr int ORDER_EXEC_WITH_PX_MSG_PRINTABLE_OFFSET
    = ORDER_EXEC_WITH_PX_MSG_OCCURRED_AT_CROSS_OFFSET + sizeof(algocor::OccurredAtCross);

static inline constexpr size_t ORDER_EXEC_WITH_PX_MSG_SIZE = ORDER_EXEC_WITH_PX_MSG_PRINTABLE_OFFSET + sizeof(algocor::Printable);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ORDER_EXEC_WITH_PX_MSG_ORDER_ID_OFFSET == 5);
static_assert(ORDER_EXEC_WITH_PX_MSG_ORDERBOOK_ID_OFFSET == 13);
static_assert(ORDER_EXEC_WITH_PX_MSG_SIDE_OFFSET == 17);
static_assert(ORDER_EXEC_WITH_PX_MSG_EXEC_QTY_OFFSET == 18);
static_assert(ORDER_EXEC_WITH_PX_MSG_MATCH_ID_OFFSET == 26);
static_assert(ORDER_EXEC_WITH_PX_MSG_COMBO_GROUP_ID_OFFSET == 34);
static_assert(ORDER_EXEC_WITH_PX_MSG_TRADE_PX_OFFSET == 52);
static_assert(ORDER_EXEC_WITH_PX_MSG_OCCURRED_AT_CROSS_OFFSET == 56);
static_assert(ORDER_EXEC_WITH_PX_MSG_PRINTABLE_OFFSET == 57);
static_assert(ORDER_EXEC_WITH_PX_MSG_SIZE == 58);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

static inline constexpr unsigned char YES = 'Y';
static inline constexpr unsigned char NO = 'N';
}  // namespace constants

struct __attribute__((packed)) OrderExecutedWithPrice {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderId order_id;
    algocor::OrderbookId orderbook_id;
    algocor::Side side;
    algocor::Quantity quantity;
    algocor::MatchId match_id;
    algocor::ComboGroupId combo_group_id;
    std::array<char, constants::ORDER_EXEC_WITH_PX_MSG_RESERVED1_SIZE> reserved1;
    std::array<char, constants::ORDER_EXEC_WITH_PX_MSG_RESERVED2_SIZE> reserved2;
    algocor::Price trade_price;
    algocor::OccurredAtCross occurred_at_cross;
    algocor::Printable printable;

    friend std::ostream& operator<<(std::ostream& os, OrderExecutedWithPrice const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10) << be32toh(order.nanoseconds)
           << ", order_id: " << be64toh(order.order_id) << ", orderbook_id: " << std::setw(10) << be32toh(order.orderbook_id)
           << ", side: " << magic_enum::enum_name(order.side) << ", quantity: " << std::setw(6) << be64toh(order.quantity)
           << ", match_id: " << be64toh(order.match_id) << ", combo_group_id: " << be64toh(order.combo_group_id)
           << ", trade_price: " << std::setw(6) << be32toh(order.trade_price)
           << ", occurred_at_cross: " << magic_enum::enum_name(order.occurred_at_cross)
           << ", printable: " << magic_enum::enum_name(order.printable) << " }\n";
        return os;
    }
};
static_assert(constants::ORDER_EXEC_WITH_PX_MSG_SIZE == sizeof(OrderExecutedWithPrice));
static_assert(std::is_trivial_v<OrderExecutedWithPrice> && std::is_standard_layout_v<OrderExecutedWithPrice>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderExecutedWithPrice> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderExecutedWithPrice>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderExecutedWithPrice> {};
