
#include "market_data_client.hpp"

#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include "../network/udp_socket.hpp"

#include "../utility/config_parser.hpp"
#include "../utility/overwrite_macros.hpp"

#include "../protocol/itch/itch_parser.hpp"

#include <magic_enum.hpp>

#include "../protocol/moldudp64/moldudp64_downstream_header.hpp"

namespace
{

[[nodiscard]] std::vector<uint64_t> populateVector(uint64_t A, uint16_t B)
{
    std::vector<uint64_t> res(B);

    int counter = 0;
    for (size_t i = A; i < A + B; ++i) {
        res[i - A] = A + counter++;
    }

    return res;
}

[[nodiscard]] std::string toStringSession(const std::array<char, 10>& token)
{
    std::string str;
    str.resize(token.size());
    for (int i = 0; i < token.size(); ++i) {
        str[i] = token[i];
    }
    return str;
}

}  // namespace

namespace algocor
{

MarketDataClient::MarketDataClient(const MarketDataPartitionConfig& config)
    : m_itchParser(*this)
    , m_config(config)
    , m_multicastSocket(*this,
          config.multicast_ip,
          config.multicast_interface_ip,
          config.multicast_port,
          config.unicast_destination_ip,
          config.unicast_destination_port)
{
    LOG_INFO("Market data client constructed. Partition name: {}, Multicast Remote: {}:{}, Local Interface: {}. Unicast Local IP: {}:{}, "
             "Unicast Remote IP: {}:{}",
        config.name,
        config.multicast_ip,
        config.multicast_port,
        config.multicast_interface_ip,
        config.unicast_request_ip,
        config.unicast_request_port,
        config.unicast_destination_ip,
        config.unicast_destination_port);
}

void MarketDataClient::run()
{
    m_multicastSocket.read();
}

[[nodiscard]] MarketDataClient::SequenceGapParseResult MarketDataClient::calculateNextSequenceNumber(uint64_t received_sequence_number,
    uint16_t message_count)
{
    SequenceGapParseResult gap = SequenceGapParseResult::NoGap;

    if (m_nextExpectedSeqNo != 0 && received_sequence_number != m_nextExpectedSeqNo) [[unlikely]] {
        gap = SequenceGapParseResult::GapDetected;
    }

    m_nextExpectedSeqNo = received_sequence_number + message_count;
    LOG_TRACE_L3("Next expected sequence number calculated as: {}", m_nextExpectedSeqNo);

    return gap;
}

[[nodiscard]] MarketDataPartitionConfig MarketDataClient::getPartitionConfig() const
{
    return m_config;
}

void MarketDataClient::setState(State state)
{
    if (m_state == state) {
        return;
    }

    const auto old_state = m_state;
    m_state = state;
    LOG_INFO("Market data client state change. {} => {}. Partition {}",
        magic_enum::enum_name(old_state),
        magic_enum::enum_name(m_state),
        m_config.name);
};

void MarketDataClient::setSessionName(const std::array<char, 10>& session_name)
{
    if (m_sessionNameSet) [[likely]] {
        return;
    }

    m_sessionName = session_name;
    m_sessionNameSet = true;
    LOG_TRACE_L3("Set session name to {} for partition {}", toStringSession(m_sessionName), m_config.name);
}

void MarketDataClient::parse(const char* buffer, size_t size)
{
    // Will be processing incoming packets here no matter what the our state is.
    const auto& [sequence_number, message_count] = m_itchParser.parse(buffer, size);

    // Rewinding.
    if (m_state == State::Rewinding) [[unlikely]] {
        LOG_TRACE_L3("State is still rewinding... Sequence number: {}, message count: {}", sequence_number, message_count);

        std::vector<uint64_t> receivedSequenceNumbers = populateVector(sequence_number, message_count);
        LOG_TRACE_L3("Received packet count while rewinding: {}", receivedSequenceNumbers.size());
        for (const auto receivedSequenceNumber : receivedSequenceNumbers) {
            LOG_TRACE_L3("Received sequence number: {}", receivedSequenceNumber);
            // TODO: there may be pathological cases where the rewound sequence numbers will not reside at the end of the vector. Think
            // about this later. scenario: another packet missed when rewinding.
            if (m_missingSequenceNumbers.back() == receivedSequenceNumber) {
                LOG_TRACE_L3("Previously requested {} sequence numbered packet is rewound successfully. Partition name: {}",
                    receivedSequenceNumber,
                    m_config.name);
                m_missingSequenceNumbers.pop_back();
            } else {
                LOG_ERROR("Rewound packet is not at the end of the missing sequence numbers vector. This means some sequence number(s) "
                          "we requested are again missing during rewinding as well. We will not take action for this yet.");
            }
        }
        // TODO: During rewind, we can still not be able to get requested packets. Handle them, too.

        if (m_missingSequenceNumbers.empty()) {
            LOG_INFO("All missing sequence numbers rewound successfully! Partition name: {}", m_config.name);
            setState(State::Continuous);
        } else {
            LOG_ERROR("There are still {} sequence numbers left not rewound yet, we will continue requesting the rest. Partition name: {}",
                m_missingSequenceNumbers.size(),
                m_config.name);

            LOG_TRACE_L3("m_lastRequestedSequenceNumber: {}, m_missingSequenceNumbers.back(): {}",
                m_lastRequestedSequenceNumber,
                m_missingSequenceNumbers.back());
            if (m_lastRequestedSequenceNumber != m_missingSequenceNumbers.back()) {
                LOG_TRACE_L3(
                    "Continuing requesting the missing packets from the rewinder starting from sequence number: {}. Partition name: {}",
                    m_missingSequenceNumbers.back(),
                    m_config.name);
                m_lastRequestedSequenceNumber = m_missingSequenceNumbers.back();
                LOG_TRACE_L3("Last req no: {}", m_lastRequestedSequenceNumber);
                requestMissingPackets(m_missingSequenceNumbers.back(),
                    m_missingSequenceNumbers.front() + 1);  // +1 since, argument is for the "RECEIVED" sequence number.
            }
        }
    } else if (m_state == State::Initial && sequence_number != 1 && m_nextExpectedSeqNo == 1) [[unlikely]] {
        // TODO: second and third condition checking might not be necessary.
        // Initial syncing. Ideally we should have used GLIMPSE here.
        LOG_TRACE_L3("Initial market data sync requested. Partition name: {}", m_config.name);

        for (uint64_t i = sequence_number - 1; i > 0; --i) {
            m_missingSequenceNumbers.push_back(i);
        }

        m_lastRequestedSequenceNumber = 1;
        m_rewindNeededTillThisSequenceNumberExclusive = sequence_number;
        LOG_TRACE_L3("Rewind is needed till this sequence number (exclusive): {}", m_rewindNeededTillThisSequenceNumberExclusive);
        requestMissingPackets(1, sequence_number);
        return;
    }

    // Check if incoming packets are missing in normal flow (not considering rewound flow).
    if (sequence_number > m_rewindNeededTillThisSequenceNumberExclusive) [[likely]] {
        LOG_TRACE_L3("sequence_number: {}, m_rewindNeededTillThisSequenceNumberExclusive: {}. This is not rewound packet, next "
                     "expected sequence number will be calculated.",
            sequence_number,
            m_rewindNeededTillThisSequenceNumberExclusive);
        if (const auto res = calculateNextSequenceNumber(sequence_number, message_count); res == SequenceGapParseResult::GapDetected) {
            requestMissingPackets(m_nextExpectedSeqNo, sequence_number);
            LOG_WARNING("Sequence number gap detected. Received sequence number: {}, Expected sequence number: {}. Partition name: {}",
                sequence_number,
                m_nextExpectedSeqNo,
                m_config.name);
        }
    } else {
        LOG_TRACE_L3("sequence_number: {}, m_rewindNeededTillThisSequenceNumberExclusive: {}. This IS a rewound packet, next "
                     "expected sequence number will NOT be calculated.",
            sequence_number,
            m_rewindNeededTillThisSequenceNumberExclusive);
    }
}

void MarketDataClient::requestMissingPackets(uint64_t expected_sequence_number, uint64_t received_sequence_number)
{
    LOG_TRACE_L3("Requesting missing UDP packets. Expected sequence number: {}, Received sequence number: {}",
        expected_sequence_number,
        received_sequence_number);
    if (received_sequence_number > expected_sequence_number) [[likely]] {
        RewindRequest rewind_request {};
        rewind_request.sequence_number = expected_sequence_number;
        rewind_request.message_count = static_cast<uint16_t>(received_sequence_number - expected_sequence_number);
        rewind(rewind_request);
    } else {
        LOG_ERROR("Missing packets detected but expected sequence number ({}) is larger than received sequence number ({}). Not "
                  "issuing an unicast request",
            expected_sequence_number,
            received_sequence_number);
    }
}

void MarketDataClient::rewind(const RewindRequest& rewind_request)
{
    setState(State::Rewinding);

    // LOG_INFO("Rewinding on ITCH client. Session name: {}. State: {} => {}", m_sessionName.data(), prev_state, m_state);
    LOG_INFO("Rewinding on ITCH client. Session name: {}", m_sessionName.data());

    protocol::moldudp64::DownstreamHeader header {};
    std::memcpy(header.session.data(), m_sessionName.data(), m_sessionName.size());

    header.sequence_number.val = htobe64(rewind_request.sequence_number);
    header.message_count.val = htobe16(rewind_request.message_count);

    LOG_TRACE_L3("Downstream header: {}", header);

    m_multicastSocket.write(&header, sizeof(protocol::moldudp64::DownstreamHeader));

    // Sync reading here. Is this ideal way to do this?
    // LOG_TRACE_L3("Waiting to read from rewinder unicast socket. Partition name: {}", m_config.name);
    // m_rewinderSocket.read(); -> this data will be multicasted from multicast socket????
    // LOG_TRACE_L3("Finished reading from rewinder unicast socket. Partition name: {}", m_config.name);
}

void MarketDataClient::processRewoundPackets(const protocol::moldudp64::DownstreamHeader& header)
{
    LOG_TRACE_L3("Processing rewound packets...");
    m_itchParser.parsePayload(reinterpret_cast<const char*>(&header), be16toh(header.message_count), be64toh(header.sequence_number));
}

}  // namespace algocor
