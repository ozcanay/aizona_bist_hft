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

static inline constexpr int COMBO_ORDERBOOK_LEG_MSG_COMBO_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int COMBO_ORDERBOOK_LEG_MSG_LEG_ORDERBOOK_ID_OFFSET
    = COMBO_ORDERBOOK_LEG_MSG_COMBO_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int COMBO_ORDERBOOK_LEG_MSG_LEG_SIDE_OFFSET
    = COMBO_ORDERBOOK_LEG_MSG_LEG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int COMBO_ORDERBOOK_LEG_MSG_LEG_RATIO_OFFSET = COMBO_ORDERBOOK_LEG_MSG_LEG_SIDE_OFFSET + sizeof(algocor::Side);

static inline constexpr size_t COMBO_ORDERBOOK_LEG_MSG_SIZE = COMBO_ORDERBOOK_LEG_MSG_LEG_RATIO_OFFSET + sizeof(algocor::LegRatio);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(COMBO_ORDERBOOK_LEG_MSG_COMBO_ORDERBOOK_ID_OFFSET == 5);
static_assert(COMBO_ORDERBOOK_LEG_MSG_LEG_ORDERBOOK_ID_OFFSET == 9);
static_assert(COMBO_ORDERBOOK_LEG_MSG_LEG_SIDE_OFFSET == 13);
static_assert(COMBO_ORDERBOOK_LEG_MSG_LEG_RATIO_OFFSET == 14);
static_assert(COMBO_ORDERBOOK_LEG_MSG_SIZE == 18);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace constants

struct __attribute__((packed)) CombinationOrderbookLeg {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId combination_orderbook_id;
    algocor::OrderbookId leg_orderbook_id;
    algocor::LegSide leg_side;
    algocor::LegRatio leg_ratio;

    friend std::ostream& operator<<(std::ostream& os, CombinationOrderbookLeg const& leg)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(leg.type) << ", nanoseconds: " << std::setw(10) << be32toh(leg.nanoseconds)
           << ", combination_orderbook_id: " << std::setw(10) << be32toh(leg.combination_orderbook_id)
           << ", leg_orderbook_id: " << std::setw(10) << be32toh(leg.leg_orderbook_id)
           << ", leg_side: " << magic_enum::enum_name(leg.leg_side) << ", leg_ratio: " << std::setw(4) << be32toh(leg.leg_ratio) << " }\n";
        return os;
    }
};
static_assert(constants::COMBO_ORDERBOOK_LEG_MSG_SIZE == sizeof(CombinationOrderbookLeg));
static_assert(std::is_trivial_v<CombinationOrderbookLeg> && std::is_standard_layout_v<CombinationOrderbookLeg>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::CombinationOrderbookLeg> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::CombinationOrderbookLeg>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::CombinationOrderbookLeg> {};
