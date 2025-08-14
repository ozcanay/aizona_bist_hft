#include "tcp_socket.hpp"

#include <arpa/inet.h>
#include <unistd.h>

#include "../utility/overwrite_macros.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

extern std::atomic_flag stopRequested;

static constexpr int KERNEL_BUFFER_SIZE
    = 16 * 1024;  // 16 KB -> While larger buffers improve throughput, they can increase latency. For low-latency applications, use smaller
                  // buffer sizes to reduce the time data spends in buffers.

namespace algocor
{

TcpSocket::TcpSocket(const std::string& host, int port)
    : m_socketFd(::socket(AF_INET, SOCK_STREAM, 0))
{
    // TODO: add error handling. make this more detailed.
    if (m_socketFd < 0) {
        LOG_ERROR("TCP socket creation failed");
        return;
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP address from text to binary form
    if (::inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0) {
        LOG_ERROR("Invalid IP address");
        ::close(m_socketFd);
        return;
    }

    if (::connect(m_socketFd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        ::close(m_socketFd);
        LOG_ERROR("TCP connection failed to {}:{}", host, port);
        return;
    }

    LOG_INFO("Connection successful to TCP server {}:{}", host, port);
}

TcpSocket::~TcpSocket()
{
    if (m_socketFd >= 0) {
        LOG_INFO("Closing TCP socket on TCP client destruction");
        ::close(m_socketFd);
    }
}

void TcpSocket::write(const char* buffer, size_t size) const
{
    size_t bytes_remaining = size;
    ssize_t bytes_sent = 0;

    while (bytes_remaining > 0 /*|| !stopRequested.test()*/) {
        bytes_sent = ::write(m_socketFd, buffer, bytes_remaining);
        if (bytes_sent < 0) {
            if (errno == EINTR) {
                LOG_TRACE_L3("Interrupted by a signal, retry");
                continue;
            }
            if (errno == EPIPE) {  // Broken pipe (connection closed by the server)
                LOG_ERROR("Write failed: connection closed by the server");
                return;  // added this.
            }

            LOG_ERROR("Write failed");
            return;  // added this.
        }
        bytes_remaining -= bytes_sent;
        buffer += bytes_sent;
    }
}

void TcpSocket::optimizeForLatency()
{
    enableNoDelay(true);
    enableQuickAck(true);
    // setSocketBufferSizes(KERNEL_BUFFER_SIZE); -> I AM NOT SURE ABOUT THIS.

    // Enable TCP Fast Open (TFO) ? -> TCP Fast Open allows data to be sent in the initial SYN packet, reducing latency for subsequent
    // connections.

    // TCP Corking?
}

void TcpSocket::enableNoDelay(bool val)
{
    // Enable TCP_NODELAY (disable Nagle's algorithm)
    const auto flag = static_cast<int>(val);
    if (setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        LOG_ERROR("Failed to set TCP_NODELAY: {}", strerror(errno));
    }
}

void TcpSocket::enableQuickAck(bool val)
{
    // Enable TCP_QUICKACK (send ACKs immediately)
    const auto flag = static_cast<int>(val);
    if (setsockopt(m_socketFd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0) {
        LOG_ERROR("Failed to set TCP_QUICKACK: {}", strerror(errno));
    }
}

void TcpSocket::setSocketBufferSizes(int bufSize)
{
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(bufSize))) {
        LOG_ERROR("Failed to set SO_RCVBUF: {}", strerror(errno));
    }
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_SNDBUF, &bufSize, sizeof(bufSize))) {
        LOG_ERROR("Failed to set SO_SNDBUF: {}", strerror(errno));
    }
}

void TcpSocket::enableBusyPolling()
{
    return;
    // I am not sure if I should be using this.

    int busyPollTime = 1;  // Time in microseconds
    if (setsockopt(m_socketFd, SOL_SOCKET, SO_BUSY_POLL, &busyPollTime, sizeof(busyPollTime))) {
        perror("Failed to enable busy polling");
    }
}

}  // namespace algocor
