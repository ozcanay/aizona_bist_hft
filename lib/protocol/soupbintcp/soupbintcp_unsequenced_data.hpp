#pragma once

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include "soupbintcp_types.hpp"
#include <magic_enum.hpp>

namespace algocor::protocol::soupbintcp
{
struct __attribute__((packed)) UnsequencedData {
    PacketLength packetLength;
    PacketType packetType;
    char data[];  // TODO: std::byte?

    friend std::ostream& operator<<(std::ostream& os, UnsequencedData const& unsequenced)
    {
        os << "{ packetLength: " << be16toh(unsequenced.packetLength)            // Convert from big-endian to host byte order
           << ", packetType: " << magic_enum::enum_name(unsequenced.packetType)  // Get enum name
           << ", raw data: \"";

        // Assuming data is a null-terminated string; adjust for your needs
        os.write(unsequenced.data, be16toh(unsequenced.packetLength) - sizeof(PacketLength) - sizeof(PacketType));
        os << "\" }\n";
        return os;
    }
};
static_assert(sizeof(UnsequencedData) == 3);
static_assert(std::is_trivial_v<UnsequencedData> && std::is_standard_layout_v<UnsequencedData>);
}  // namespace algocor::protocol::soupbintcp

template<>
struct fmtquill::formatter<algocor::protocol::soupbintcp::UnsequencedData> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::soupbintcp::UnsequencedData>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::soupbintcp::UnsequencedData> {};
