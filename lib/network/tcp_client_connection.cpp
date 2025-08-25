#include "tcp_client_connection.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace algocor
{

TcpClientConnection::TcpClientConnection(int socket_fd)
    : m_socketFd(socket_fd)
{
    optimizeForLatency();
}

TcpClientConnection::~TcpClientConnection()
{
    if (m_socketFd >= 0) {
        ::close(m_socketFd);
        LOG_TRACE_L3("Client connection closed: fd {}", m_socketFd);
    }
}

void TcpClientConnection::write(const char* buffer, size_t size)
{
    size_t bytes_remaining = size;
    ssize_t bytes_sent = 0;

    while (bytes_remaining > 0) {
        bytes_sent = ::write(m_socketFd, buffer, bytes_remaining);
        if (bytes_sent < 0) {
            if (errno == EINTR) {
                LOG_TRACE_L3("Write interrupted by signal, retry");
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_TRACE_L3("Write would block, retry");
                continue;
            }
            if (errno == EPIPE) {
                LOG_INFO("Write failed: connection closed by client {}", m_socketFd);
                return;
            }
            LOG_ERROR("Write failed to client {}: {}", m_socketFd, strerror(errno));
            return;
        }
        bytes_remaining -= bytes_sent;
        buffer += bytes_sent;
    }
}

void TcpClientConnection::optimizeForLatency()
{
    enableNoDelay(true);
    enableQuickAck(true);
}

void TcpClientConnection::enableNoDelay(bool val)
{
    int flag = static_cast<int>(val);
    if (::setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        LOG_ERROR("Failed to set TCP_NODELAY on client {}: {}", m_socketFd, strerror(errno));
    }
}

void TcpClientConnection::enableQuickAck(bool val)
{
    int flag = static_cast<int>(val);
    if (::setsockopt(m_socketFd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag)) < 0) {
        LOG_ERROR("Failed to set TCP_QUICKACK on client {}: {}", m_socketFd, strerror(errno));
    }
}

}  // namespace algocor