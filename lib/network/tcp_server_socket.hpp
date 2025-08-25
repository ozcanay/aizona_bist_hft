#pragma once

namespace algocor
{

class TcpServerSocket {
public:
    explicit TcpServerSocket(int port);
    ~TcpServerSocket();

    int acceptConnection();
    int getSocketFd() const
    {
        return m_socketFd;
    }

private:
    int m_socketFd = -1;
    int m_port;

    void optimizeForLatency();
    void enableReuseAddr(bool val);
};

}  // namespace algocor