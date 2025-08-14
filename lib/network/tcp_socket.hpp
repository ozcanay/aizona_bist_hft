#pragma once

#include <string>
#include <unistd.h>

#include "../utility/overwrite_macros.hpp"

namespace algocor
{

// TODO: should I include socket non-blocking setting?

class TcpSocket {
public:
    explicit TcpSocket(const std::string& host, int port);
    ~TcpSocket();

    template<size_t N>
    [[nodiscard]] ssize_t read(std::array<char, N>& buffer, size_t& offset) const
    {
        ssize_t bytesRead = ::read(m_socketFd, buffer.data() + offset, buffer.size() - offset);
        if (bytesRead < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LOG_ERROR("Read error: {}", strerror(errno));
            }
            return bytesRead;
        }
        if (bytesRead == 0) {
            LOG_ERROR("Connection closed by peer");
        }
        offset += bytesRead;
        return bytesRead;
    }

    void write(const char* buffer, size_t size) const;
    void optimizeForLatency();

private:
    int m_socketFd = -1;

    void enableNoDelay(bool val);
    void enableQuickAck(bool val);
    void setSocketBufferSizes(int bufSize);
    void enableBusyPolling();
};

static_assert(sizeof(TcpSocket) == 4);

}  // namespace algocor
