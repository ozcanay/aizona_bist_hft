#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) LoginRejected {
    PacketLength packetLength;
    PacketType packetType;
    RejectReasonCode reject_reason_code;

    friend std::ostream& operator<<(std::ostream& os, LoginRejected const& loginRejected)
    {
        os << "{ packetLength: " << loginRejected.packetLength << ", packetType: '" << magic_enum::enum_name(loginRejected.packetType)
           << "'"
           << ", reject_reason_code: " << magic_enum::enum_name(loginRejected.reject_reason_code) << " }";
        return os;
    }
};
static_assert(sizeof(LoginRejected) == 4);
static_assert(std::is_trivial_v<LoginRejected> && std::is_standard_layout_v<LoginRejected>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::LoginRejected> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::LoginRejected>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::LoginRejected> {};
