#include "tcp_server_socket.hpp"

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../utility/overwrite_macros.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace algocor
{

TcpServerSocket::TcpServerSocket(int port)
    : m_port(port)
    , m_socketFd(::socket(AF_INET, SOCK_STREAM, 0))
{
    if (m_socketFd < 0) {
        LOG_ERROR("Failed to create server socket: {}", strerror(errno));
        throw std::runtime_error("Failed to create server socket");
    }

    enableReuseAddr(true);
    optimizeForLatency();

    // Set server socket to non-blocking
    int flags = ::fcntl(m_socketFd, F_GETFL, 0);
    if (flags < 0 || ::fcntl(m_socketFd, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_ERROR("Failed to set server socket non-blocking: {}", strerror(errno));
        ::close(m_socketFd);
        throw std::runtime_error("Failed to set server socket non-blocking");
    }

    sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (::bind(m_socketFd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
        LOG_ERROR("Failed to bind server socket to port {}: {}", port, strerror(errno));
        ::close(m_socketFd);
        throw std::runtime_error("Failed to bind server socket");
    }

    if (::listen(m_socketFd, SOMAXCONN) < 0) {
        LOG_ERROR("Failed to listen on server socket: {}", strerror(errno));
        ::close(m_socketFd);
        throw std::runtime_error("Failed to listen on server socket");
    }

    LOG_INFO("Server socket bound to port {}", port);
}

TcpServerSocket::~TcpServerSocket()
{
    if (m_socketFd >= 0) {
        ::close(m_socketFd);
        LOG_INFO("Server socket closed");
    }
}

int TcpServerSocket::acceptConnection()
{
    sockaddr_in client_addr {};
    socklen_t client_len = sizeof(client_addr);

    int client_fd = ::accept(m_socketFd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        return -1;  // Caller should check errno
    }

    char client_ip[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    LOG_INFO("Accepted connection from {}:{} on fd {}", client_ip, ntohs(client_addr.sin_port), client_fd);

    return client_fd;
}

void TcpServerSocket::optimizeForLatency()
{
    // Enable TCP_NODELAY (disable Nagle's algorithm)
    int flag = 1;
    if (::setsockopt(m_socketFd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
        LOG_ERROR("Failed to set TCP_NODELAY on server socket: {}", strerror(errno));
    }
}

void TcpServerSocket::enableReuseAddr(bool val)
{
    int flag = static_cast<int>(val);
    if (::setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        LOG_ERROR("Failed to set SO_REUSEADDR on server socket: {}", strerror(errno));
    }
}

}  // namespace algocor