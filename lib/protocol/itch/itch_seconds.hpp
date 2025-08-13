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

static inline constexpr size_t SECONDS_MSG_SIZE = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::Second);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(SECONDS_MSG_SIZE == 5);
}  // namespace constants

struct __attribute__((packed)) Seconds {
    algocor::MessageType type;
    algocor::TimestampNanoseconds second;

    friend std::ostream& operator<<(std::ostream& os, Seconds const& seconds)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(seconds.type) << ", second: " << std::setw(10) << be32toh(seconds.second) << " }\n";
        return os;
    }
};
static_assert(constants::SECONDS_MSG_SIZE == sizeof(Seconds));
static_assert(std::is_trivial_v<Seconds> && std::is_standard_layout_v<Seconds>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::Seconds> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::Seconds> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::Seconds> {};
