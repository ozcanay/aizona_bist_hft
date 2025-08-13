#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

namespace algocor
{

// reference: https://borsaistanbul.com/files/soupbintcp-protocol-specification24F25E31E16B2FE7E78531A3.pdf
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

enum class PacketType : char
{
    Debug = '+',
    LoginAccepted = 'A',
    LoginRejected = 'J',
    SequencedData = 'S',
    ServerHeartbeat = 'H',
    EndOfSession = 'Z',
    LoginRequest = 'L',
    UnsequencedData = 'U',
    ClientHeartbeat = 'R',
    LogoutRequest = 'O',
};
static_assert(sizeof(PacketType) == 1);

using PacketLength = uint16_t;
static_assert(sizeof(PacketLength) == 2 && std::is_arithmetic_v<PacketLength>);

enum class RejectReasonCode : char
{
    NotAuthorized = 'A',
    SessionNotAvailable = 'S',
};
static_assert(sizeof(RejectReasonCode) == 1);

using Username = std::array<char, 6>;
using Password = std::array<char, 10>;
using SessionName = std::array<char, 10>;  // TODO: move this.
using SequenceNumberByteArray = std::array<char, 20>;

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace algocor
