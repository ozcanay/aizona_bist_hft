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

class TcpClientConnection {
public:
    explicit TcpClientConnection(int socket_fd);
    ~TcpClientConnection();

    template<size_t N>
    [[nodiscard]] ssize_t read(std::array<char, N>& buffer, size_t& offset)
    {
        ssize_t bytesRead = ::read(m_socketFd, buffer.data() + offset, buffer.size() - offset);
        if (bytesRead < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_ERROR("Read error from client {}: {}", m_socketFd, strerror(errno));
            }
            return bytesRead;
        }
        if (bytesRead == 0) {
            LOG_INFO("Connection closed by client {}", m_socketFd);
            return bytesRead;
        }
        offset += bytesRead;
        return bytesRead;
    }

    void write(const char* buffer, size_t size);
    int getSocketFd() const
    {
        return m_socketFd;
    }

private:
    int m_socketFd = -1;

    void optimizeForLatency();
    void enableNoDelay(bool val);
    void enableQuickAck(bool val);
};

}  // namespace algocor