#pragma once

#include "types.hpp"

// TODO: read all of ITCH doc, and add any missing value etc.

namespace algocor
{

// TODO: these belong to enum class Side : unsigned char.
// static inline constexpr unsigned char SIDE_BUY = 'B';
// static inline constexpr unsigned char SIDE_SELL = 'S';

static inline constexpr int MSG_TYPE_OFFSET = 0;
static inline constexpr int TIMESTAMP_NANOSECONDS_OFFSET = MSG_TYPE_OFFSET + sizeof(algocor::MessageType);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(TIMESTAMP_NANOSECONDS_OFFSET == 1);

}  // namespace algocor
