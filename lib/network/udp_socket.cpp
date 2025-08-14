#include "udp_socket.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

// #include "client/market_data_client.hpp"

#include <array>
#include <atomic>

extern std::atomic_flag stopRequested;

namespace algocor
{

UdpMulticastSocket::UdpMulticastSocket(MarketDataClient& market_data_client,
    const std::string& multicast_ip,
    const std::string& interface_ip,
    int multicast_port,
    const std::string& unicast_destination_ip,
    int unicast_destination_port)
    : m_marketDataClient(market_data_client)
    , m_socketFd(::socket(AF_INET, SOCK_DGRAM, 0))
{
    if (m_socketFd == -1) {
        throw std::runtime_error("Socket creation failed: " + std::string(strerror(errno)));
    }

    int reuse = 1;
    if (::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        throw std::runtime_error("setsockopt SO_REUSEADDR failed: " + std::string(strerror(errno)));
    }

    // Bind to multicast port
    sockaddr_in local_addr {};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(multicast_port);

    if (::bind(m_socketFd, reinterpret_cast<sockaddr*>(&local_addr), sizeof(local_addr)) < 0) {
        throw std::runtime_error("Bind failed: " + std::string(strerror(errno)));
    }

    // Join multicast group
    ip_mreq mreq {};
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip.c_str());
    mreq.imr_interface.s_addr = inet_addr(interface_ip.c_str());
    if (setsockopt(m_socketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        throw std::runtime_error("setsockopt IP_ADD_MEMBERSHIP failed: " + std::string(strerror(errno)));
    }

    // Set explicit multicast interface
    struct in_addr local_interface {};
    local_interface.s_addr = inet_addr(interface_ip.c_str());
    if (setsockopt(m_socketFd, IPPROTO_IP, IP_MULTICAST_IF, &local_interface, sizeof(local_interface)) < 0) {
        throw std::runtime_error("setsockopt IP_MULTICAST_IF failed: " + std::string(strerror(errno)));
    }

    // Set up unicast destination
    m_unicastDestAddr.sin_family = AF_INET;
    m_unicastDestAddr.sin_port = htons(unicast_destination_port);
    if (inet_pton(AF_INET, unicast_destination_ip.c_str(), &m_unicastDestAddr.sin_addr) != 1) {
        throw std::runtime_error("Invalid unicast destination address");
    }

    // m_marketDataClient.setState(MarketDataClient::State::Joined);
}

UdpMulticastSocket::~UdpMulticastSocket()
{
    // LOG_TRACE_L3("Destructing UDP multicast socket. Partition: {}", m_marketDataClient.getPartitionConfig().name);
    if (m_socketFd != -1) {
        ::close(m_socketFd);
        // LOG_TRACE_L3("Closing UDP multicast socket. Partition: {}", m_marketDataClient.getPartitionConfig().name);
    }
}

void UdpMulticastSocket::read() const
{
    std::array<char, 2048> buffer {};  // maybe increase the size of this buffer. maybe make this a member field.
    sockaddr_in sender_addr {};
    socklen_t sender_addr_len = sizeof(sender_addr);

    while (!stopRequested.test()) {
        // LOG_TRACE_L3("Reading from UDP multicast socket... Partition: {}", m_marketDataClient.getPartitionConfig().name);
        ssize_t bytes_received
            = ::recvfrom(m_socketFd, buffer.data(), buffer.size(), 0, reinterpret_cast<sockaddr*>(&sender_addr), &sender_addr_len);
        // LOG_TRACE_L3(
        //     "=> Read {} bytes from UDP multicast socket. Partition: {}", bytes_received, m_marketDataClient.getPartitionConfig().name);

        if (bytes_received > 0) {
            // m_marketDataClient.parse(buffer.data(), bytes_received);
        } else {
            // LOG_ERROR("Receiving from UDP multicast socket failed: {}. Partition: {}",
            //     strerror(errno),
            //     m_marketDataClient.getPartitionConfig().name);
            break;
        }
    }
}

void UdpMulticastSocket::write(const void* buffer, size_t size)
{
    ssize_t sentBytes = sendto(m_socketFd, buffer, size, 0, reinterpret_cast<sockaddr*>(&m_unicastDestAddr), sizeof(m_unicastDestAddr));
    if (sentBytes == -1) {
        // LOG_ERROR("UDP multicast socket send error: {}. Partition name: {}", strerror(errno),
        // m_marketDataClient.getPartitionConfig().name);
    } else {
        // LOG_TRACE_L3("<= Sent {} bytes to unicast destination. Partition name: {}", size, m_marketDataClient.getPartitionConfig().name);
    }
}

}  // namespace algocor
