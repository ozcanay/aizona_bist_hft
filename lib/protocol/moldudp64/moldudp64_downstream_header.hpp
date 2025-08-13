#pragma once

#include "../../types.hpp"

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::moldudp64
{

struct __attribute__((packed)) DownstreamHeader {
    algocor::SessionName session;
    algocor::SequenceNumber sequence_number;
    algocor::MessageCount message_count;

    // NOLINTNEXTLINE(hicpp-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
    char data[0];  // TODO: I do not like this at all.

    friend std::ostream& operator<<(std::ostream& os, DownstreamHeader const& downstreamHeader)
    {
        os << "{ session: \"" << std::string(downstreamHeader.session.data(), downstreamHeader.session.size()) << "\""
           << ", sequence_number: \"" << be64toh(downstreamHeader.sequence_number) << "\""
           << ", message_count: " << be16toh(downstreamHeader.message_count) << " }";  // Assuming little-endian format for message_count
        return os;
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
static_assert(sizeof(DownstreamHeader) == 20);
static_assert(std::is_trivial_v<DownstreamHeader> && std::is_standard_layout_v<DownstreamHeader>);

}  // namespace algocor::protocol::moldudp64

template<>
struct fmtquill::formatter<algocor::protocol::moldudp64::DownstreamHeader> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::moldudp64::DownstreamHeader>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::moldudp64::DownstreamHeader> {};
