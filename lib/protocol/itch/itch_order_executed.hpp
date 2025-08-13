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

static inline constexpr int ORDER_EXEC_MSG_RESERVED1_SIZE = 7;
static inline constexpr int ORDER_EXEC_MSG_RESERVED2_SIZE = 7;

static inline constexpr int ORDER_EXEC_MSG_ORDER_ID_OFFSET = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ORDER_EXEC_MSG_ORDERBOOK_ID_OFFSET = ORDER_EXEC_MSG_ORDER_ID_OFFSET + sizeof(algocor::OrderId);
static inline constexpr int ORDER_EXEC_MSG_SIDE_OFFSET = ORDER_EXEC_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int ORDER_EXEC_MSG_EXEC_QTY_OFFSET = ORDER_EXEC_MSG_SIDE_OFFSET + sizeof(algocor::Side);
static inline constexpr int ORDER_EXEC_MSG_MATCH_ID_OFFSET = ORDER_EXEC_MSG_EXEC_QTY_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int ORDER_EXEC_MSG_COMBO_GROUP_ID_OFFSET = ORDER_EXEC_MSG_MATCH_ID_OFFSET + sizeof(algocor::MatchId);

static inline constexpr size_t ORDER_EXEC_MSG_SIZE
    = ORDER_EXEC_MSG_COMBO_GROUP_ID_OFFSET + sizeof(algocor::ComboGroupId) + ORDER_EXEC_MSG_RESERVED1_SIZE + ORDER_EXEC_MSG_RESERVED2_SIZE;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ORDER_EXEC_MSG_ORDER_ID_OFFSET == 5);
static_assert(ORDER_EXEC_MSG_ORDERBOOK_ID_OFFSET == 13);
static_assert(ORDER_EXEC_MSG_SIDE_OFFSET == 17);
static_assert(ORDER_EXEC_MSG_EXEC_QTY_OFFSET == 18);
static_assert(ORDER_EXEC_MSG_MATCH_ID_OFFSET == 26);
static_assert(ORDER_EXEC_MSG_COMBO_GROUP_ID_OFFSET == 34);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace constants

struct __attribute__((packed)) OrderExecuted {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderId order_id;
    algocor::OrderbookId orderbook_id;
    algocor::Side side;
    algocor::Quantity quantity;
    algocor::MatchId match_id;
    algocor::ComboGroupId combo_group_id;
    std::array<char, 7> reserved1;
    std::array<char, 7> reserved2;

    friend std::ostream& operator<<(std::ostream& os, OrderExecuted const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10) << be32toh(order.nanoseconds)
           << ", order_id: " << be64toh(order.order_id) << ", orderbook_id: " << std::setw(10) << be32toh(order.orderbook_id)
           << ", side: " << magic_enum::enum_name(order.side) << ", quantity: " << std::setw(6) << be64toh(order.quantity)
           << ", match_id: " << be64toh(order.match_id) << ", combo_group_id: " << be64toh(order.combo_group_id) << " }\n";
        return os;
    }
};
static_assert(constants::ORDER_EXEC_MSG_SIZE == sizeof(OrderExecuted));
static_assert(std::is_trivial_v<OrderExecuted> && std::is_standard_layout_v<OrderExecuted>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderExecuted> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderExecuted> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderExecuted> {};
