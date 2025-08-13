#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) ServerHeartbeat {
    PacketLength packetLength;
    PacketType packetType;

    friend std::ostream& operator<<(std::ostream& os, ServerHeartbeat const& serverHeartbeat)
    {
        os << "{ packetLength: " << serverHeartbeat.packetLength << ", packetType: '" << magic_enum::enum_name(serverHeartbeat.packetType)
           << "' }";
        return os;
    }
};
static_assert(sizeof(ServerHeartbeat) == 3);
static_assert(std::is_trivial_v<ServerHeartbeat> && std::is_standard_layout_v<ServerHeartbeat>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::ServerHeartbeat> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::ServerHeartbeat>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::ServerHeartbeat> {};
