#include "tcp_client.hpp"

#include <arpa/inet.h>
#include <array>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#include "../protocol/ouch/ouch_order_accepted.hpp"
#include "../protocol/ouch/ouch_order_canceled.hpp"
#include "../protocol/ouch/ouch_order_executed.hpp"
#include "../protocol/ouch/ouch_order_rejected.hpp"
#include "../protocol/ouch/ouch_order_replaced.hpp"
#include "../protocol/soupbintcp/soupbintcp_client_heartbeat.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_accepted.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_rejected.hpp"
#include "../protocol/soupbintcp/soupbintcp_sequenced_data.hpp"

#include "../client/order_entry_client.hpp"
#include "../utility/overwrite_macros.hpp"

extern std::atomic_flag stopRequested;

template<typename T>
constexpr std::array<char, sizeof(T)> toByteArray(const T& obj)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    return std::bit_cast<std::array<char, sizeof(T)>>(obj);
}

namespace algocor
{

TcpClient::TcpClient(BistMarketAccessor& market_accessor, const std::string& host, int port)
    : m_tcpSocket(TcpSocket(host, port))
    , m_marketAccessor(market_accessor)
    , m_dataSize(0)
{
}

void TcpClient::startRead()
{
    LOG_INFO("Starting reading from TCP socket");
    while (!stopRequested.test()) {
        ssize_t bytesRead = readFromSocket();
        LOG_TRACE_L1("Read {} bytes from TCP socket", bytesRead);
        if (bytesRead <= 0) {
            break;
        }
        processBuffer();
    }
}

ssize_t TcpClient::readFromSocket()
{
    // TODO: is it possible that we will try to read more than the exchange sends?
    return m_tcpSocket.read(m_buffer, m_dataSize);
    // think about how we are doing this at bull-tech. -> I AM PROBABLY BEING DUMB. SINCE READ OPERATION RETURNS AMOUNT OF BYTES READ, THERE
    // SHOULD BE NO PROBLEM.
}

void TcpClient::processBuffer()
{
    size_t offset = 0;
    while (m_dataSize - offset >= sizeof(uint16_t)) {
        uint16_t messageLength = 0;
        std::memcpy(&messageLength, m_buffer.data() + offset, sizeof(uint16_t));
        messageLength = be16toh(messageLength);

        LOG_TRACE_L3("messageLength: {}, offset: {}, m_dataSize: {}", messageLength, offset, m_dataSize);

        if (m_dataSize - offset < sizeof(uint16_t) + messageLength) {
            LOG_TRACE_L1("Incomplete message, will be waiting for more data");
            break;  // Incomplete message, wait for more data
        }

        const char* messageData = m_buffer.data() + offset;
        handleMessage(messageData, sizeof(uint16_t) + messageLength);
        offset += sizeof(uint16_t) + messageLength;
    }

    // Move leftover data to the front of the buffer
    if (offset > 0) {
        std::memmove(m_buffer.data(), m_buffer.data() + offset, m_dataSize - offset);
        m_dataSize -= offset;
    }
}

void TcpClient::handleMessage(const char* data, uint16_t length)
{
    LOG_TRACE_L1("Handling message of length {}", length);

    const auto* sequenced_data = reinterpret_cast<const protocol::soupbintcp::SequencedData*>(data);

    uint16_t packet_length = be16toh(sequenced_data->packetLength);
    const auto packet_type = sequenced_data->packetType;

    if (packet_type == PacketType::SequencedData) [[likely]] {
        const auto payload_type = static_cast<protocol::ouch::MessageType>(sequenced_data->data[0]);
        if (payload_type == protocol::ouch::MessageType::OrderCanceled) {
            protocol::ouch::OrderCanceled order_canceled {};
            std::memcpy(&order_canceled, data + sizeof(protocol::soupbintcp::UnsequencedData), length - sizeof(PacketType));

            order_canceled.order_id.val = be64toh(order_canceled.order_id);
            order_canceled.timestamp.val = be64toh(order_canceled.timestamp);

            LOG_TRACE_L3("=> Order canceled received: {}", order_canceled);
        } else if (payload_type == protocol::ouch::MessageType::OrderAccepted) {
            protocol::ouch::OrderAccepted order_accepted {};
            std::memcpy(&order_accepted, data + sizeof(protocol::soupbintcp::UnsequencedData), length - sizeof(PacketType));
            // TODO: What if I only parse the fields I am interested in, not all the bytes.

            // order_accepted.orderbook_id.val = be32toh(order_accepted.orderbook_id);
            // order_accepted.order_id.val = be64toh(order_accepted.order_id);
            // order_accepted.display_quantity.val = be64toh(order_accepted.display_quantity);
            // order_accepted.timestamp.val = be64toh(order_accepted.timestamp);
            // order_accepted.price.val = be32toh(order_accepted.price);
            // order_accepted.quantity.val = be64toh(order_accepted.quantity);
            // order_accepted.pre_trade_quantity.val = be64toh(order_accepted.pre_trade_quantity);

            LOG_TRACE_L3("=> Order accepted received: {}", order_accepted);

            // not byte swapping orderbook ID to improve latency.
            m_marketAccessor.m_orderManager.orderAccepted(order_accepted.order_token,
                be64toh(order_accepted.quantity),
                order_accepted.orderbook_id,
                be32toh(order_accepted.price),
                static_cast<char>(order_accepted.side));

            // for testing.
            m_marketAccessor.sendReplaceOrder(order_accepted.order_token, 775'000);

        } else if (payload_type == protocol::ouch::MessageType::OrderExecuted) {
            protocol::ouch::OrderExecuted order_executed {};
            std::memcpy(&order_executed, data + sizeof(protocol::soupbintcp::UnsequencedData), length - sizeof(PacketType));

            // order_executed.orderbook_id.val = be32toh(order_executed.orderbook_id.val);
            // order_executed.timestamp.val = be64toh(order_executed.timestamp);
            // order_executed.trade_price.val = be32toh(order_executed.trade_price);
            // order_executed.traded_quantity.val = be64toh(order_executed.traded_quantity);

            LOG_TRACE_L3("=> Order executed received: {}", order_executed);

            m_marketAccessor.m_orderManager.orderExecuted(order_executed.order_token, be64toh(order_executed.traded_quantity));
        } else if (payload_type == protocol::ouch::MessageType::OrderReplaced) {
            protocol::ouch::OrderReplaced order_replaced {};
            std::memcpy(&order_replaced, data + sizeof(protocol::soupbintcp::UnsequencedData), length - sizeof(PacketType));

            // order_replaced.timestamp.val = be64toh(order_replaced.timestamp);
            // order_replaced.orderbook_id.val = be32toh(order_replaced.orderbook_id);
            // order_replaced.order_id.val = be64toh(order_replaced.order_id);
            // order_replaced.display_quantity.val = be64toh(order_replaced.display_quantity);
            // order_replaced.price.val = be32toh(order_replaced.price);
            // order_replaced.quantity.val = be64toh(order_replaced.quantity);
            // order_replaced.pre_trade_quantity.val = be64toh(order_replaced.pre_trade_quantity);

            LOG_TRACE_L3("=> Order replaced received: {}", order_replaced);

            // TODO: Fix this shit.
            // m_marketAccessor.m_orderManager.orderReplaced(order_replaced.replacement_order_token, be32toh(order_replaced.price));

            // for testing.
            m_marketAccessor.sendCancelOrder(order_replaced.replacement_order_token);
        } else if (payload_type == protocol::ouch::MessageType::OrderRejected) {
            protocol::ouch::OrderRejected order_rejected {};
            std::memcpy(&order_rejected, data + sizeof(protocol::soupbintcp::UnsequencedData), length - sizeof(PacketType));

            order_rejected.timestamp.val = be64toh(order_rejected.timestamp);
            order_rejected.reject_code
                = static_cast<RejectCode>(be32toh(static_cast<std::underlying_type_t<RejectCode>>(order_rejected.reject_code)));

            // if(const auto iter = OUCH_REJECT_CODE_MAP.find(order_rejected.reject_code)) {

            // }

            LOG_TRACE_L3("=> Order rejected received: {}", order_rejected);

            m_marketAccessor.m_orderManager.orderDeleted(order_rejected.order_token);
        } else {
            LOG_ERROR("=> Unexpected payload type ({}) in sequenced message", static_cast<char>(payload_type));
        }
    } else if (packet_type == PacketType::ServerHeartbeat) {
        LOG_TRACE_L3("=> Server heartbeat received");

        protocol::soupbintcp::ClientHeartbeat client_heartbeat {};
        client_heartbeat.packetType = PacketType::ClientHeartbeat;
        client_heartbeat.packetLength = be16toh(sizeof(PacketType));

        const auto& byte_array = toByteArray(client_heartbeat);

        write(byte_array.data(), byte_array.size());
        LOG_TRACE_L3("<= Sent heartbeat");
    } else if (packet_type == PacketType::LoginAccepted) {
        LOG_INFO("=> Login accepted");
        protocol::soupbintcp::LoginAccepted login_accepted {};  // TODO: change struct make it similar to order responses.
        std::memcpy(&login_accepted, data, length);
        m_marketAccessor.onLoginAccepted(login_accepted);
    } else if (packet_type == PacketType::LoginRejected) {
        LOG_ERROR("=> Login rejected");
        protocol::soupbintcp::LoginRejected login_rejected {};  // TODO: change struct make it similar to order responses.
        std::memcpy(&login_rejected, data, length);
        std::cout << "reject reason code: " << static_cast<unsigned int>(login_rejected.reject_reason_code) << std::endl;
    } else if (packet_type == PacketType::EndOfSession) {
        LOG_INFO("=> End of session");
    } else {
        LOG_ERROR("Unidentified packet received. Packet type: {}", static_cast<char>(packet_type));
    }
}

void TcpClient::write(const char* buffer, size_t size) const
{
    LOG_TRACE_L1("TCP Client writing to tcp socket. Size: {}", size);
    m_tcpSocket.write(buffer, size);
}

}  // namespace algocor
