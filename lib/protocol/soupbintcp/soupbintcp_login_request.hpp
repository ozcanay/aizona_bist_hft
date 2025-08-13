#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) LoginRequest {
    PacketLength packetLength;
    PacketType packetType;
    Username username;
    Password password;
    SessionName sessionName;
    SequenceNumberByteArray sequenceNumber;

    friend std::ostream& operator<<(std::ostream& os, LoginRequest const& request)
    {
        os << "{ packetLength: " << be16toh(request.packetLength) << ", packetType: " << magic_enum::enum_name(request.packetType)
           << ", username: \"" << std::string(request.username.data(), request.username.size()) << "\""
           << ", password: \"" << std::string(request.password.data(), request.password.size()) << "\""
           << ", sessionName: \"" << std::string(request.sessionName.data(), request.sessionName.size()) << "\""
           << ", sequenceNumber: \"" << std::string(request.sequenceNumber.data(), request.sequenceNumber.size()) << "\""
           << " }\n";
        return os;
    }
};
static_assert(sizeof(LoginRequest) == 49);
static_assert(std::is_trivial_v<LoginRequest> && std::is_standard_layout_v<LoginRequest>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::LoginRequest> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::LoginRequest>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::LoginRequest> {};
