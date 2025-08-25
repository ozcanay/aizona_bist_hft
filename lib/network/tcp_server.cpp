#include "tcp_server.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <memory>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include "tcp_client_connection.hpp"
#include "tcp_server_socket.hpp"

namespace algocor
{

TcpServer::TcpServer(int port, size_t max_events)
    : m_port(port)
    , m_maxEvents(max_events)
    , m_events(max_events)
{
    m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
    if (m_epollFd < 0) {
        LOG_ERROR("Failed to create epoll fd: {}", strerror(errno));
        throw std::runtime_error("Failed to create epoll");
    }

    LOG_INFO("TCP server created for port {}", m_port);
}

TcpServer::~TcpServer()
{
    stop();

    if (m_epollFd >= 0) {
        ::close(m_epollFd);
        LOG_INFO("Epoll fd closed");
    }
}

void TcpServer::start()
{
    if (m_running.load()) {
        LOG_WARNING("TCP server already running");
        return;
    }

    m_serverSocket = std::make_unique<TcpServerSocket>(m_port);

    // Add server socket to epoll
    epoll_event event {};
    event.events = EPOLLIN | EPOLLET;  // Edge-triggered
    event.data.fd = m_serverSocket->getSocketFd();

    if (::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_serverSocket->getSocketFd(), &event) < 0) {
        LOG_ERROR("Failed to add server socket to epoll: {}", strerror(errno));
        throw std::runtime_error("Failed to add server socket to epoll");
    }

    m_running.store(true);
    m_serverThread = std::thread([this]() { serverLoop(); });

    LOG_INFO("TCP server started on port {}", m_port);
}

void TcpServer::stop()
{
    if (!m_running.load()) {
        return;
    }

    m_running.store(false);

    if (m_serverThread.joinable()) {
        m_serverThread.join();
    }

    // Close all client connections
    for (auto& [fd, client] : m_clients) {
        client.reset();
    }
    m_clients.clear();

    m_serverSocket.reset();

    LOG_INFO("TCP server stopped");
}

void TcpServer::serverLoop()
{
    LOG_INFO("Server loop started");

    while (m_running.load()) {
        int event_count = ::epoll_wait(m_epollFd, m_events.data(), m_maxEvents, 100);  // 100ms timeout

        if (event_count < 0) {
            if (errno == EINTR) {
                continue;
            }
            LOG_ERROR("Epoll wait failed: {}", strerror(errno));
            break;
        }

        for (int i = 0; i < event_count; ++i) {
            const auto& event = m_events[i];

            if (event.data.fd == m_serverSocket->getSocketFd()) {
                // New connection
                handleNewConnection();
            } else {
                // Client data
                if (event.events & EPOLLIN) {
                    handleClientData(event.data.fd);
                }
                if (event.events & (EPOLLHUP | EPOLLERR)) {
                    LOG_INFO("Client {} disconnected (EPOLLHUP/EPOLLERR)", event.data.fd);
                    removeClient(event.data.fd);
                }
            }
        }
    }

    LOG_INFO("Server loop ended");
}

void TcpServer::handleNewConnection()
{
    while (true) {  // Accept all pending connections (edge-triggered)
        int client_fd = m_serverSocket->acceptConnection();
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more connections to accept
            }
            LOG_ERROR("Failed to accept connection: {}", strerror(errno));
            break;
        }

        // Set client socket to non-blocking
        int flags = ::fcntl(client_fd, F_GETFL, 0);
        if (flags < 0 || ::fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            LOG_ERROR("Failed to set client socket non-blocking: {}", strerror(errno));
            ::close(client_fd);
            continue;
        }

        auto client_connection = std::make_unique<TcpClientConnection>(client_fd);
        m_clients[client_fd] = std::move(client_connection);

        addClientToEpoll(client_fd);

        LOG_INFO("New client connected: fd {}", client_fd);

        if (m_connectionHandler) {
            m_connectionHandler(client_fd, true);
        }
    }
}

void TcpServer::handleClientData(int client_fd)
{
    auto it = m_clients.find(client_fd);
    if (it == m_clients.end()) {
        LOG_WARNING("Received data for unknown client fd {}", client_fd);
        return;
    }

    static constexpr size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> buffer {};
    size_t offset = 0;

    while (true) {  // Read all available data (edge-triggered)
        ssize_t bytes_read = it->second->read(buffer, offset);

        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more data to read
            }
            LOG_ERROR("Read error from client {}: {}", client_fd, strerror(errno));
            removeClient(client_fd);
            return;
        }

        if (bytes_read == 0) {
            LOG_INFO("Client {} disconnected", client_fd);
            removeClient(client_fd);
            return;
        }

        // Process complete messages here
        if (m_messageHandler && offset > 0) {
            m_messageHandler(client_fd, buffer.data(), offset);
        }

        offset = 0;  // Reset for next read
    }
}

void TcpServer::addClientToEpoll(int client_fd)
{
    epoll_event event {};
    event.events = EPOLLIN | EPOLLET;  // Edge-triggered
    event.data.fd = client_fd;

    if (::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, client_fd, &event) < 0) {
        LOG_ERROR("Failed to add client {} to epoll: {}", client_fd, strerror(errno));
    }
}

void TcpServer::removeClient(int client_fd)
{
    auto it = m_clients.find(client_fd);
    if (it != m_clients.end()) {
        if (m_connectionHandler) {
            m_connectionHandler(client_fd, false);
        }

        ::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, client_fd, nullptr);
        it->second.reset();
        m_clients.erase(it);

        LOG_INFO("Client {} removed", client_fd);
    }
}

void TcpServer::sendToClient(int client_fd, const char* buffer, size_t size)
{
    auto it = m_clients.find(client_fd);
    if (it != m_clients.end()) {
        it->second->write(buffer, size);
    } else {
        LOG_WARNING("Attempted to send to unknown client fd {}", client_fd);
    }
}

void TcpServer::broadcastToAll(const char* buffer, size_t size)
{
    for (const auto& [fd, client] : m_clients) {
        client->write(buffer, size);
    }
    LOG_TRACE_L3("Broadcasted message to {} clients", m_clients.size());
}

void TcpServer::disconnectClient(int client_fd)
{
    removeClient(client_fd);
}

}  // namespace algocor