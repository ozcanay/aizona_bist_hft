#pragma once

#include "../../types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

// #include "sequenced_data.hpp"

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) LoginAccepted {
    PacketLength packetLength;
    PacketType packetType;
    SessionName session_name;
    SequenceNumberByteArray sequenceNumber;

    friend std::ostream& operator<<(std::ostream& os, LoginAccepted const& loginAccepted)
    {
        os << "{ packetLength: " << loginAccepted.packetLength << ", packetType: '" << magic_enum::enum_name(loginAccepted.packetType)
           << "'"
           << ", session_name: \"" << std::string(loginAccepted.session_name.data(), loginAccepted.session_name.size()) << "\""
           << ", sequenceNumber: \"" << std::string(loginAccepted.sequenceNumber.data(), loginAccepted.sequenceNumber.size()) << "\""
           << " }";
        return os;
    }
};
static_assert(sizeof(LoginAccepted) == 33);
static_assert(std::is_trivial_v<LoginAccepted> && std::is_standard_layout_v<LoginAccepted>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::LoginAccepted> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::LoginAccepted>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::LoginAccepted> {};
