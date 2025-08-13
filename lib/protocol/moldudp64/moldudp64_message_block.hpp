#pragma once

#include "../../types.hpp"

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::moldudp64
{

struct __attribute__((packed)) MessageBlock {
    algocor::MessageLength length;

    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    char data[];  // TODO
                  // In Standard C and C++, zero-size array is not allowed. So I did not specify 0 as the length.

    friend std::ostream& operator<<(std::ostream& os, MessageBlock const& messageBlock)
    {
        os << "{ length: " << be32toh(messageBlock.length) << " }";  // Assuming little-endian format for length
        return os;
    }
};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
static_assert(sizeof(MessageBlock) == 2);
static_assert(std::is_trivial_v<MessageBlock> && std::is_standard_layout_v<MessageBlock>);
}  // namespace algocor::protocol::moldudp64

template<>
struct fmtquill::formatter<algocor::protocol::moldudp64::MessageBlock> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::moldudp64::MessageBlock>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::moldudp64::MessageBlock> {};
