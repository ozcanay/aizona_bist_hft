#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) ClientHeartbeat {
    PacketLength packetLength;
    PacketType packetType;

    friend std::ostream& operator<<(std::ostream& os, ClientHeartbeat const& heartbeat)
    {
        os << "{ packetLength: " << be32toh(heartbeat.packetLength) << ", packetType: " << magic_enum::enum_name(heartbeat.packetType)
           << " }\n";
        return os;
    }
};
static_assert(sizeof(ClientHeartbeat) == 3);
static_assert(std::is_trivial_v<ClientHeartbeat> && std::is_standard_layout_v<ClientHeartbeat>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::ClientHeartbeat> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::ClientHeartbeat>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::ClientHeartbeat> {};
