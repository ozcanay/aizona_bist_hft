#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

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

static inline constexpr int ORDERBOOK_FLUSH_MSG_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ORDERBOOK_FLUSH_MSG_SIZE = ORDERBOOK_FLUSH_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ORDERBOOK_FLUSH_MSG_ORDERBOOK_ID_OFFSET == 5);
static_assert(ORDERBOOK_FLUSH_MSG_SIZE == 9);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) OrderbookFlush {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId orderbook_id;

    friend std::ostream& operator<<(std::ostream& os, OrderbookFlush const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", nanoseconds: " << std::setw(10) << be32toh(order.nanoseconds)
           << ", orderbook_id: " << std::setw(10) << be32toh(order.orderbook_id) << " }\n";
        return os;
    }
};

static_assert(constants::ORDERBOOK_FLUSH_MSG_SIZE == sizeof(OrderbookFlush));
static_assert(std::is_trivial_v<OrderbookFlush> && std::is_standard_layout_v<OrderbookFlush>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderbookFlush> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderbookFlush> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderbookFlush> {
};
