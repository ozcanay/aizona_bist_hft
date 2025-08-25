#pragma once

#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "../network/udp_socket.hpp"

#include "../utility/config_parser.hpp"
#include "../utility/overwrite_macros.hpp"

#include "../core/orderbook_builder.hpp"
#include "../protocol/itch/itch_parser.hpp"

// TODO: ADD A CODE TO SANITY CHECK THAT ALL SEQUENCE NUMBERS UP TO THIS POINT ARE RECEIVED. ENABLE THIS ONLY FOR DEBUG BUILDS.
namespace algocor
{

namespace protocol::moldudp64
{
struct DownstreamHeader;
}

class MarketDataClient {
    struct RewindRequest {
        uint64_t sequence_number;
        uint16_t message_count;
    };

    enum class SequenceGapParseResult : std::uint8_t
    {
        NoGap,
        GapDetected,
    };

public:
    explicit MarketDataClient(const MarketDataPartitionConfig& config);
    void run();
    [[nodiscard]] SequenceGapParseResult calculateNextSequenceNumber(uint64_t received_sequence_number, uint16_t message_count);
    [[nodiscard]] MarketDataPartitionConfig getPartitionConfig() const;

private:
    protocol::itch::ConcreteOrderbookBuilder m_builder;
    protocol::itch::ItchParser<protocol::itch::ConcreteOrderbookBuilder> m_itchParser;
    MarketDataPartitionConfig m_config;
    UdpMulticastSocket m_multicastSocket;
    // UdpUnicastSocket m_rewinderSocket;
    std::vector<uint64_t> m_missingSequenceNumbers;
    uint64_t m_lastRequestedSequenceNumber { 0 };

    // TODO: I need to make this robust. I am currently not making use of this.
    uint64_t m_rewindNeededTillThisSequenceNumberExclusive { 0 };

    std::array<char, 10> m_sessionName {};
    bool m_sessionNameSet { false };

    uint64_t m_nextExpectedSeqNo = 1;  // first sequence number to receive will be 1, not 0.

    enum class State
    {
        Initial,
        Joined,
        Snapshotting,  // TODO: this needs GLIMPSE.
        Rewinding,
        Continuous,
    } m_state
        = State::Initial;

    void setState(State state);
    void setSessionName(const std::array<char, 10>& session_name);
    void parse(const char* buffer, size_t size);
    void requestMissingPackets(uint64_t expected_sequence_number, uint64_t received_sequence_number);
    void rewind(const RewindRequest& rewind_request);
    void processRewoundPackets(const struct protocol::moldudp64::DownstreamHeader& header);

    friend class ::algocor::protocol::itch::ItchParser<protocol::itch::ConcreteOrderbookBuilder>;
    friend class UdpMulticastSocket;
};

}  // namespace algocor
