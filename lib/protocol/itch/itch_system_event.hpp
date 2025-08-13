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

static inline constexpr int SYSTEM_EVENT_MSG_EVENT_CODE_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);

static inline constexpr size_t SYSTEM_EVENT_MSG_SIZE = SYSTEM_EVENT_MSG_EVENT_CODE_OFFSET + sizeof(algocor::EventCode);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(SYSTEM_EVENT_MSG_EVENT_CODE_OFFSET == 5);
static_assert(SYSTEM_EVENT_MSG_SIZE == 6);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) SystemEvent {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::EventCode event_code;

    friend std::ostream& operator<<(std::ostream& os, SystemEvent const& event)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(event.type) << ", nanoseconds: " << std::setw(10) << be32toh(event.nanoseconds)
           << ", event_code: " << magic_enum::enum_name(event.event_code) << " }\n";
        return os;
    }
};
static_assert(constants::SYSTEM_EVENT_MSG_SIZE == sizeof(SystemEvent));
static_assert(std::is_trivial_v<SystemEvent> && std::is_standard_layout_v<SystemEvent>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::SystemEvent> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::SystemEvent> : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::SystemEvent> {};
