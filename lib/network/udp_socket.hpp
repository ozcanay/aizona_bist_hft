#pragma once

#include <arpa/inet.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

#include <string>

namespace algocor
{

class UdpMulticastSocket {
public:
    explicit UdpMulticastSocket(class MarketDataClient& market_data_client,
        const std::string& multicast_ip,
        const std::string& interface_ip,
        int multicast_port,
        const std::string& unicast_destination_ip,
        int unicast_destination_port);

    ~UdpMulticastSocket();
    void read() const;
    void write(const void* buffer, size_t size);

private:
    class MarketDataClient& m_marketDataClient;
    int m_socketFd = -1;
    struct sockaddr_in m_unicastDestAddr {};  // Unicast destination address
};

}  // namespace algocor
