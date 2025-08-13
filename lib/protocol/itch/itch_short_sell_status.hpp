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

static inline constexpr int SHORT_SELL_MSG_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int SHORT_SELL_MSG_RESTRICTION_OFFSET = SHORT_SELL_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int SHORT_SELL_MSG_VALIDATION_OFFSET = SHORT_SELL_MSG_RESTRICTION_OFFSET + sizeof(uint8_t);

static inline constexpr size_t SHORT_SELL_MSG_SIZE = SHORT_SELL_MSG_VALIDATION_OFFSET + sizeof(uint8_t);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(SHORT_SELL_MSG_ORDERBOOK_ID_OFFSET == 5);
static_assert(SHORT_SELL_MSG_RESTRICTION_OFFSET == 9);
static_assert(SHORT_SELL_MSG_VALIDATION_OFFSET == 10);
static_assert(SHORT_SELL_MSG_SIZE == 11);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) ShortSellStatus {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId orderbook_id;
    algocor::ShortSellRestriction short_sell_restriction;
    algocor::ShortSellValidation short_sell_validation;

    friend std::ostream& operator<<(std::ostream& os, ShortSellStatus const& status)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(status.type) << ", nanoseconds: " << std::setw(10) << be32toh(status.nanoseconds)
           << ", orderbook_id: " << std::setw(10) << be32toh(status.orderbook_id)
           << ", short_sell_restriction: " << magic_enum::enum_name(status.short_sell_restriction)
           << ", short_sell_validation: " << magic_enum::enum_name(status.short_sell_validation) << " }\n";
        return os;
    }
};
static_assert(constants::SHORT_SELL_MSG_SIZE == sizeof(ShortSellStatus));
static_assert(std::is_trivial_v<ShortSellStatus> && std::is_standard_layout_v<ShortSellStatus>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::ShortSellStatus> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::ShortSellStatus>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::ShortSellStatus> {};
