#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <new>  // For std::hardware_destructive_interference_size
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>

#include "../network/tcp_client.hpp"

#include "../utility/config_parser.hpp"

#include "../core/order_manager.hpp"
#include "../protocol/ouch/ouch_cancel_order.hpp"
#include "../protocol/ouch/ouch_enter_order.hpp"
#include "../protocol/ouch/ouch_replace_order.hpp"
#include "../protocol/soupbintcp/soupbintcp_unsequenced_data.hpp"

namespace algocor
{

namespace protocol::soupbintcp
{
struct LoginAccepted;
struct LoginRejected;
}  // namespace protocol::soupbintcp

class BistMarketAccessor {
public:
    explicit BistMarketAccessor(OrderEntryPartitionConfig partition,
        std::string client_account,
        std::string exchange_info,
        int last_order_token_index);
    ~BistMarketAccessor();
    void login();
    void logout();

    void sendEnterOrder(uint32_t orderbook_id, char side, uint32_t qty, int32_t price);
    void sendReplaceOrder(const std::array<char, 14>& original_order_token, int32_t price);
    void sendCancelOrder(const std::array<char, 14>& original_order_token);

private:
    OrderEntryPartitionConfig m_partition;
    std::string m_clientAccount;
    std::string m_exchangeInfo;

    std::string m_partitionDefinition;
    TcpClient m_tcpClient;

    OrderManager m_orderManager;

    uint64_t m_sequenceNumber = 0;
    std::string m_sessionName;
    std::thread m_tcpClientThread;

    enum class State
    {
        Continuous,
        Disconnected,
        Reconnecting,
    } m_state
        = State::Disconnected;

    // https://stackoverflow.com/questions/39680206/understanding-stdhardware-destructive-interference-size-and-stdhardware-cons
    // TODO: std::hardware_destructive_interference_size -> this produced compiler warnings. fix it later.
    alignas(/*std::hardware_destructive_interference_size*/ 64)
        std::array<char, sizeof(protocol::soupbintcp::UnsequencedData) + sizeof(protocol::ouch::EnterOrder)> m_enterOrderByteArray {};
    alignas(/*std::hardware_destructive_interference_size*/ 64)
        std::array<char, sizeof(protocol::soupbintcp::UnsequencedData) + sizeof(protocol::ouch::ReplaceOrder)> m_replaceOrderByteArray {};
    alignas(/*std::hardware_destructive_interference_size*/ 64)
        std::array<char, sizeof(protocol::soupbintcp::UnsequencedData) + sizeof(protocol::ouch::CancelOrder)> m_cancelOrderByteArray {};

    void onLoginAccepted(struct protocol::soupbintcp::LoginAccepted& login_accepted);
    void onLoginRejected(struct protocol::soupbintcp::LoginRejected& login_rejected);

    void prepareEnterOrderBuffer();
    void prepareReplaceOrderBuffer();
    void prepareCancelOrderBuffer();

    friend class TcpClient;
};

}  // namespace algocor
