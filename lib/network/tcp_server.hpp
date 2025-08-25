#pragma once

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

namespace algocor
{

class TcpServerSocket;
class TcpClientConnection;

using MessageHandler = std::function<void(int client_fd, const char* data, size_t length)>;
using ConnectionHandler = std::function<void(int client_fd, bool connected)>;

class TcpServer {
public:
    explicit TcpServer(int port, size_t max_events = 1024);
    ~TcpServer();

    void setMessageHandler(MessageHandler handler)
    {
        m_messageHandler = std::move(handler);
    }
    void setConnectionHandler(ConnectionHandler handler)
    {
        m_connectionHandler = std::move(handler);
    }

    void start();
    void stop();

    void sendToClient(int client_fd, const char* buffer, size_t size);
    void broadcastToAll(const char* buffer, size_t size);
    void disconnectClient(int client_fd);

private:
    int m_port;
    size_t m_maxEvents;
    int m_epollFd = -1;
    std::unique_ptr<TcpServerSocket> m_serverSocket;
    std::atomic<bool> m_running { false };
    std::thread m_serverThread;

    std::unordered_map<int, std::unique_ptr<TcpClientConnection>> m_clients;
    std::vector<epoll_event> m_events;

    MessageHandler m_messageHandler;
    ConnectionHandler m_connectionHandler;

    void serverLoop();
    void handleNewConnection();
    void handleClientData(int client_fd);
    void removeClient(int client_fd);
    void addClientToEpoll(int client_fd);
};

}  // namespace algocor