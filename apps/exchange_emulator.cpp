#include "../../lib/utility/quill_wrapper.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_accepted.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_rejected.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_request.hpp"
#include "../protocol/soupbintcp/soupbintcp_types.hpp"
#include "tcp_server.hpp"
#include <cstring>
#include <endian.h>
#include <iostream>

std::atomic_flag stopRequested = ATOMIC_FLAG_INIT;

template<size_t N>
std::string trimString(const std::array<char, N>& arr)
{
    std::string str(arr.begin(), arr.end());
    str.erase(str.find_last_not_of(' ') + 1);
    return str;
}

template<size_t N>
std::array<char, N> toPaddedArray(const std::string& input, char pad = ' ')
{
    std::array<char, N> arr {};
    std::fill(arr.begin(), arr.end(), pad);
    std::copy_n(input.begin(), std::min(input.size(), arr.size()), arr.begin());
    return arr;
}

void handleSoupBinTcpMessage(algocor::TcpServer& server, int client_fd, const char* data, size_t length)
{
    using namespace algocor;
    using namespace algocor::protocol::soupbintcp;

    if (length < 3) {  // Minimum: packet_length(2) + packet_type(1)
        LOG_ERROR("Message too short from client {}: {} bytes", client_fd, length);
        return;
    }

    // Parse packet length (first 2 bytes, big endian)
    uint16_t packet_length = 0;
    std::memcpy(&packet_length, data, sizeof(uint16_t));
    packet_length = be16toh(packet_length);

    LOG_INFO("Received packet from client {}: length={}, total_received={}", client_fd, packet_length, length);

    if (length < sizeof(uint16_t) + packet_length) {
        LOG_ERROR("Incomplete packet from client {}", client_fd);
        return;
    }

    // Get packet type (3rd byte)
    PacketType packet_type = static_cast<PacketType>(data[2]);
    LOG_INFO("Packet type from client {}: {}", client_fd, static_cast<char>(packet_type));

    switch (packet_type) {
        case PacketType::LoginRequest: {
            LOG_INFO("Processing login request from client {}", client_fd);

            if (length < sizeof(LoginRequest)) {
                LOG_ERROR("Invalid login request size from client {}: expected {}, got {}", client_fd, sizeof(LoginRequest), length);
                return;
            }

            // Parse the complete LoginRequest
            const auto* login_req = reinterpret_cast<const LoginRequest*>(data);
            LOG_INFO("=> Login request received: {}", *login_req);

            // Extract and trim username/password
            std::string username = trimString(login_req->username);
            std::string password = trimString(login_req->password);
            std::string session_name = trimString(login_req->sessionName);

            LOG_INFO("Parsed credentials from client {}: username='{}', password='{}', session='{}'",
                client_fd,
                username,
                password,
                session_name);

            // Check credentials
            if (username == "TEST" && password == "TEST_PWD") {
                // Send LoginAccepted
                LoginAccepted login_accepted {};
                login_accepted.packetLength = htobe16(sizeof(LoginAccepted) - sizeof(PacketLength));
                login_accepted.packetType = PacketType::LoginAccepted;

                // Set session name (use the one from request or generate new one)
                if (session_name.empty()) {
                    session_name = "SES_" + std::to_string(client_fd);
                }
                login_accepted.session_name = toPaddedArray<10>(session_name);

                // Set sequence number (start from 1)
                login_accepted.sequenceNumber = toPaddedArray<20>("1");

                LOG_INFO("<= Sending login accepted to client {}: {}", client_fd, login_accepted);

                server.sendToClient(client_fd, reinterpret_cast<const char*>(&login_accepted), sizeof(login_accepted));

            } else {
                // Send LoginRejected
                LOG_WARNING("Invalid credentials from client {}: username='{}', password='{}'", client_fd, username, password);

                // You'll need to define LoginRejected struct, but for now just disconnect
                LOG_INFO("Disconnecting client {} due to invalid credentials", client_fd);
                server.disconnectClient(client_fd);
            }
            break;
        }

        case PacketType::ClientHeartbeat: {
            LOG_TRACE_L3("=> Client heartbeat received from {}", client_fd);
            // Echo back heartbeat or handle as needed
            break;
        }

        case PacketType::LogoutRequest: {
            LOG_INFO("=> Logout request received from client {}", client_fd);
            server.disconnectClient(client_fd);
            break;
        }

        case PacketType::UnsequencedData: {
            LOG_INFO("=> Unsequenced data received from client {}, length={}", client_fd, packet_length);
            // Handle OUCH messages here
            break;
        }

        default: {
            LOG_ERROR("Unknown packet type from client {}: {}", client_fd, static_cast<char>(packet_type));
            break;
        }
    }
}

int main()
{
    setup_quill("exchange_emulator.txt", quill::LogLevel::TraceL3);

    algocor::TcpServer server(6642);

    // Set message handler to parse SoupBinTCP protocol
    server.setMessageHandler(
        [&server](int client_fd, const char* data, size_t length) { handleSoupBinTcpMessage(server, client_fd, data, length); });

    // Set connection handler
    server.setConnectionHandler([](int client_fd, bool connected) {
        if (connected) {
            LOG_INFO("Client {} connected", client_fd);
            std::cout << "Client " << client_fd << " connected" << std::endl;
        } else {
            LOG_INFO("Client {} disconnected", client_fd);
            std::cout << "Client " << client_fd << " disconnected" << std::endl;
        }
    });

    server.start();
    std::cout << "SoupBinTCP server started on port 6642" << std::endl;

    // Keep running
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}