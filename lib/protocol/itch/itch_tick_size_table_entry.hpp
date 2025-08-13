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

static inline constexpr int TICK_SIZE_MSG_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int TICK_SIZE_MSG_TICK_SIZE_OFFSET = TICK_SIZE_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int TICK_SIZE_MSG_PRICE_FROM_OFFSET = TICK_SIZE_MSG_TICK_SIZE_OFFSET + sizeof(algocor::TickSize);
static inline constexpr int TICK_SIZE_MSG_PRICE_TO_OFFSET = TICK_SIZE_MSG_PRICE_FROM_OFFSET + sizeof(algocor::Price);

static inline constexpr size_t TICK_SIZE_MSG_SIZE = TICK_SIZE_MSG_PRICE_TO_OFFSET + sizeof(algocor::Price);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(TICK_SIZE_MSG_ORDERBOOK_ID_OFFSET == 5);
static_assert(TICK_SIZE_MSG_TICK_SIZE_OFFSET == 9);
static_assert(TICK_SIZE_MSG_PRICE_FROM_OFFSET == 17);
static_assert(TICK_SIZE_MSG_PRICE_TO_OFFSET == 21);
static_assert(TICK_SIZE_MSG_SIZE == 25);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) TickSizeTableEntry {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId orderbook_id;
    algocor::TickSize tick_size;
    algocor::Price price_from;
    algocor::Price price_to;

    friend std::ostream& operator<<(std::ostream& os, TickSizeTableEntry const& entry)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(entry.type) << ", nanoseconds: " << std::setw(10) << be32toh(entry.nanoseconds)
           << ", orderbook_id: " << std::setw(10) << be32toh(entry.orderbook_id) << ", tick_size: " << std::setw(6)
           << be64toh(entry.tick_size) << ", price_from: " << std::setw(10) << be32toh(entry.price_from) << ", price_to: " << std::setw(10)
           << be32toh(entry.price_to) << " }\n";
        return os;
    }
};
static_assert(constants::TICK_SIZE_MSG_SIZE == sizeof(TickSizeTableEntry));
static_assert(std::is_trivial_v<TickSizeTableEntry> && std::is_standard_layout_v<TickSizeTableEntry>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::TickSizeTableEntry> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::TickSizeTableEntry>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::TickSizeTableEntry> {};