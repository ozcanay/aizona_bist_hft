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

static inline constexpr int ORDERBOOK_STATE_MSG_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ORDERBOOK_STATE_MSG_STATE_NAME_OFFSET = ORDERBOOK_STATE_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);

static constexpr int STATE_NAME_LENGTH = 20;
static inline constexpr size_t ORDERBOOK_STATE_MSG_SIZE = ORDERBOOK_STATE_MSG_STATE_NAME_OFFSET + STATE_NAME_LENGTH;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ORDERBOOK_STATE_MSG_ORDERBOOK_ID_OFFSET == 5);
static_assert(ORDERBOOK_STATE_MSG_STATE_NAME_OFFSET == 9);
static_assert(ORDERBOOK_STATE_MSG_SIZE == 29);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) OrderBookState {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId orderbook_id;
    std::array<char, constants::STATE_NAME_LENGTH> state_name;

    friend std::ostream& operator<<(std::ostream& os, OrderBookState const& state)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(state.type) << ", nanoseconds: " << std::setw(10) << be32toh(state.nanoseconds)
           << ", orderbook_id: " << std::setw(10) << be32toh(state.orderbook_id) << ", state_name: \""
           << std::string(state.state_name.data(), state.state_name.size()) << "\""
           << " }\n";
        return os;
    }
};

static_assert(constants::ORDERBOOK_STATE_MSG_SIZE == sizeof(OrderBookState));
static_assert(std::is_trivial_v<OrderBookState> && std::is_standard_layout_v<OrderBookState>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderBookState> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderBookState> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderBookState> {
};
