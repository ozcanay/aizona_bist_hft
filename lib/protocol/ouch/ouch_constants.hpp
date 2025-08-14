#pragma once

#include "../soupbintcp/soupbintcp_types.hpp"

namespace algocor
{

// Message types
/*
 * These messages can be sent by both the client and server.
 */
static constexpr char PACKET_TYPE_DEBUG = '+';

/*
 * These messages are sent by the server to the client.
 */
inline static constexpr char PACKET_TYPE_LOGIN_ACCEPTED = 'A';
inline static constexpr char PACKET_TYPE_LOGIN_REJECTED = 'J';
inline static constexpr char PACKET_TYPE_SEQUENCED_DATA = 'S';
inline static constexpr char PACKET_TYPE_SERVER_HEARTBEAT = 'H';
inline static constexpr char PACKET_TYPE_END_OF_SESSION = 'Z';

/*
 * These messages are sent by the client to the server.
 */
inline static constexpr char PACKET_TYPE_LOGIN_REQUEST = 'L';
inline static constexpr char PACKET_TYPE_UNSEQUENCED_DATA = 'U';
inline static constexpr char PACKET_TYPE_CLIENT_HEARTBEAT = 'R';
inline static constexpr char PACKET_TYPE_LOGOUT_REQUEST = 'O';

inline static constexpr int MAX_PACKET_LENGTH = 65'535;

inline static constexpr char LOGIN_REJECT_CODE_NOT_AUTHORIZED = 'A';
inline static constexpr char LOGIN_REJECT_CODE_SESSION_NOT_AVAILABLE = 'S';

// reference: https://borsaistanbul.com/files/soupbintcp-protocol-specification24F25E31E16B2FE7E78531A3.pdf
inline static constexpr int USERNAME_LENGTH = 6;
inline static constexpr int PASSWORD_LENGTH = 10;
inline static constexpr int SESSION_NAME_LENGTH = 10;
inline static constexpr int REQUESTED_SEQUENCE_NUMBER_LENGTH = 20;

inline static constexpr int LOGIN_REQUEST_PACKET_LENGTH
    = sizeof(PacketType) + USERNAME_LENGTH + PASSWORD_LENGTH + SESSION_NAME_LENGTH + REQUESTED_SEQUENCE_NUMBER_LENGTH;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(LOGIN_REQUEST_PACKET_LENGTH == 47);

}  // namespace algocor
