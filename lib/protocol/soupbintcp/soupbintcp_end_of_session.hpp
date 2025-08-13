#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) EndOfSession {
    PacketLength packetLength;
    PacketType packetType;

    friend std::ostream& operator<<(std::ostream& os, EndOfSession const& endOfSession)
    {
        os << "{ packetLength: " << endOfSession.packetLength << ", packetType: '" << magic_enum::enum_name(endOfSession.packetType)
           << "' }";
        return os;
    }
};
static_assert(sizeof(EndOfSession) == 3);
static_assert(std::is_trivial_v<EndOfSession> && std::is_standard_layout_v<EndOfSession>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::EndOfSession> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::EndOfSession>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::EndOfSession> {};
