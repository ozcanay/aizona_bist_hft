#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "tcp_socket.hpp"

namespace algocor
{

class TcpClient {
public:
    explicit TcpClient(class BistMarketAccessor& market_accessor, const std::string& host, int port);

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
    TcpClient(TcpClient&& other) noexcept = delete;
    TcpClient& operator=(TcpClient&& other) noexcept = delete;

    void startRead();
    void write(const char* buffer, size_t size) const;

private:
    TcpSocket m_tcpSocket;

    class BistMarketAccessor& m_marketAccessor;

    // TODO: think about if performance would benefit from a larger buffer size.
    static inline constexpr size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> m_buffer {};
    size_t m_dataSize;

    [[nodiscard]] ssize_t readFromSocket();
    void processBuffer();
    void handleMessage(const char* data, uint16_t length);
};

}  // namespace algocor
