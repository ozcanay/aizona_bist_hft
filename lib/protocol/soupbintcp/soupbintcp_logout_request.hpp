#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) LogoutRequest {
    PacketLength packetLength;
    PacketType packetType;

    friend std::ostream& operator<<(std::ostream& os, LogoutRequest const& request)
    {
        os << "{ packetLength: " << be16toh(request.packetLength) << ", packetType: " << magic_enum::enum_name(request.packetType)
           << " }\n";
        return os;
    }
};
static_assert(sizeof(LogoutRequest) == 3);
static_assert(std::is_trivial_v<LogoutRequest> && std::is_standard_layout_v<LogoutRequest>);

}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::LogoutRequest> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::LogoutRequest>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::LogoutRequest> {};
