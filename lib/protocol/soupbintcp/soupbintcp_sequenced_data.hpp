#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) SequencedData {
    PacketLength packetLength;
    PacketType packetType;
    char data[];

    friend std::ostream& operator<<(std::ostream& os, SequencedData const& sequencedData)
    {
        os << "{ packetLength: " << be16toh(sequencedData.packetLength) << ", packetType: '"
           << magic_enum::enum_name(sequencedData.packetType) << "' }";
        return os;
    }
};
static_assert(sizeof(SequencedData) == 3);
static_assert(std::is_trivial_v<SequencedData> && std::is_standard_layout_v<SequencedData>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::SequencedData> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::SequencedData>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::SequencedData> {};
