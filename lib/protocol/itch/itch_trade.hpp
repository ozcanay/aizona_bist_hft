#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

#include <array>
#include <cstddef>

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <iomanip>

#include <magic_enum.hpp>

namespace algocor::protocol::itch
{

namespace constants
{

static inline constexpr int TRADE_MSG_RESERVED1_SIZE = 7;
static inline constexpr int TRADE_MSG_RESERVED2_SIZE = 7;

static inline constexpr int TRADE_MSG_MATCH_ID_OFFSET = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int TRADE_MSG_COMBO_GROUP_ID_OFFSET = TRADE_MSG_MATCH_ID_OFFSET + sizeof(algocor::MatchId);
static inline constexpr int TRADE_MSG_SIDE_OFFSET = TRADE_MSG_COMBO_GROUP_ID_OFFSET + sizeof(algocor::ComboGroupId);
static inline constexpr int TRADE_MSG_QUANTITY_OFFSET = TRADE_MSG_SIDE_OFFSET + sizeof(algocor::Side);
static inline constexpr int TRADE_MSG_ORDERBOOK_ID_OFFSET = TRADE_MSG_QUANTITY_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int TRADE_MSG_TRADE_PRICE_OFFSET = TRADE_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int TRADE_MSG_PRINTABLE_OFFSET
    = TRADE_MSG_TRADE_PRICE_OFFSET + sizeof(algocor::Price) + TRADE_MSG_RESERVED1_SIZE + TRADE_MSG_RESERVED2_SIZE;
static inline constexpr int TRADE_MSG_OCCURRED_AT_CROSS_OFFSET = TRADE_MSG_PRINTABLE_OFFSET + sizeof(algocor::Printable);
static inline constexpr size_t TRADE_MSG_SIZE = TRADE_MSG_OCCURRED_AT_CROSS_OFFSET + sizeof(algocor::OccurredAtCross);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(TRADE_MSG_MATCH_ID_OFFSET == 5);
static_assert(TRADE_MSG_COMBO_GROUP_ID_OFFSET == 13);
static_assert(TRADE_MSG_SIDE_OFFSET == 17);
static_assert(TRADE_MSG_QUANTITY_OFFSET == 18);
static_assert(TRADE_MSG_ORDERBOOK_ID_OFFSET == 26);
static_assert(TRADE_MSG_TRADE_PRICE_OFFSET == 30);
static_assert(TRADE_MSG_PRINTABLE_OFFSET == 48);
static_assert(TRADE_MSG_OCCURRED_AT_CROSS_OFFSET == 49);
static_assert(TRADE_MSG_SIZE == 50);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

static inline constexpr unsigned char SIDE_ANON = ' ';
}  // namespace constants

struct __attribute__((packed)) Trade {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::MatchId match_id;
    algocor::ComboGroupId combo_group_id;
    algocor::Side side;
    algocor::Quantity quantity;
    algocor::OrderbookId orderbook_id;
    algocor::Price trade_price;
    std::array<char, constants::TRADE_MSG_RESERVED1_SIZE> reserved1;
    std::array<char, constants::TRADE_MSG_RESERVED2_SIZE> reserved2;
    algocor::Printable printable;
    algocor::OccurredAtCross occurred_at_cross;

    friend std::ostream& operator<<(std::ostream& os, Trade const& trade)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(trade.type) << ", nanoseconds: " << std::setw(10) << be32toh(trade.nanoseconds)
           << ", match_id: " << std::setw(10) << be64toh(trade.match_id) << ", combo_group_id: " << std::setw(10)
           << be32toh(trade.combo_group_id) << ", side: " << magic_enum::enum_name(trade.side) << ", quantity: " << std::setw(6)
           << be64toh(trade.quantity) << ", orderbook_id: " << std::setw(10) << be32toh(trade.orderbook_id)
           << ", trade_price: " << std::setw(10) << be32toh(trade.trade_price) << ", printable: " << magic_enum::enum_name(trade.printable)
           << ", occurred_at_cross: " << magic_enum::enum_name(trade.occurred_at_cross) << " }\n";
        return os;
    }
};
static_assert(constants::TRADE_MSG_SIZE == sizeof(Trade));
static_assert(std::is_trivial_v<Trade> && std::is_standard_layout_v<Trade>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::Trade> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::Trade> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::Trade> {};
