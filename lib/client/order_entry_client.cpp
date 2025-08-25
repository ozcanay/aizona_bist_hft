#include "order_entry_client.hpp"

#include "../protocol/ouch/ouch_constants.hpp"
#include "../protocol/ouch/ouch_enter_order.hpp"
#include "../protocol/ouch/ouch_types.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_accepted.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_rejected.hpp"
#include "../protocol/soupbintcp/soupbintcp_login_request.hpp"
#include "../protocol/soupbintcp/soupbintcp_logout_request.hpp"
#include "../protocol/soupbintcp/soupbintcp_unsequenced_data.hpp"

#include <array>
#include <bit>
#include <cstring>
#include <endian.h>
#include <type_traits>

#include "../utility/overwrite_macros.hpp"
#include <magic_enum.hpp>

template<size_t N>
std::array<char, N> toPaddedArray(const std::string& input, char pad = ' ')
{
    std::array<char, N> arr {};
    std::fill(arr.begin(), arr.end(), pad);
    std::copy_n(input.begin(), std::min(input.size(), arr.size()), arr.begin());
    return arr;
}

template<typename T>
constexpr std::array<char, sizeof(T)> toByteArray(const T& obj)
{
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    return std::bit_cast<std::array<char, sizeof(T)>>(obj);
}

// template<typename T>
// constexpr std::array<char, sizeof(T)> toByteArray(const T& obj)
// {
//     static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
//     std::array<char, sizeof(T)> bytes {};
//     std::memcpy(bytes.data(), &obj, sizeof(T));
//     return bytes;
// }

namespace algocor
{

BistMarketAccessor::BistMarketAccessor(OrderEntryPartitionConfig partition,
    std::string client_account,
    std::string exchange_info,
    int last_order_token_index)
    : m_partition(std::move(partition))
    , m_clientAccount(std::move(client_account))
    , m_exchangeInfo(std::move(exchange_info))
    , m_partitionDefinition(m_partition.toString())
    , m_tcpClient(*this, m_partition.ip, m_partition.port)
    , m_orderManager(last_order_token_index)
{
    prepareEnterOrderBuffer();
    prepareReplaceOrderBuffer();
    prepareCancelOrderBuffer();

    m_tcpClientThread = std::thread([this]() { m_tcpClient.startRead(); });
    LOG_INFO("Constructing OUCH client for partition: {}, client account: {}, exchange info: {}",
        m_partitionDefinition,
        m_clientAccount,
        m_exchangeInfo);
}

BistMarketAccessor::~BistMarketAccessor()
{
    logout();

    LOG_TRACE_L1("Destructing OUCH client for partition: {}", m_partitionDefinition);
    if (m_tcpClientThread.joinable()) {
        LOG_TRACE_L1("Joining TCP client thread for partition: {}", m_partitionDefinition);
        m_tcpClientThread.join();
    } else {
        LOG_WARNING("TCP client thread was not joinable for partition: {}", m_partitionDefinition);
    }
    LOG_TRACE_L1("Destructed OUCH client for partition: {}", m_partitionDefinition);
}

void BistMarketAccessor::login()
{
    protocol::soupbintcp::LoginRequest login_request {};
    login_request.packetLength = htobe16(LOGIN_REQUEST_PACKET_LENGTH);
    login_request.packetType = PacketType::LoginRequest;
    login_request.username = toPaddedArray<USERNAME_LENGTH>(m_partition.username);
    login_request.password = toPaddedArray<PASSWORD_LENGTH>(m_partition.password);
    login_request.sessionName = toPaddedArray<SESSION_NAME_LENGTH>(m_sessionName);
    login_request.sequenceNumber = toPaddedArray<REQUESTED_SEQUENCE_NUMBER_LENGTH>(std::to_string(m_sequenceNumber + 1));

    const auto& byte_array = toByteArray(login_request);

    m_tcpClient.write(byte_array.data(), byte_array.size());
    LOG_INFO("<= Sent login request for partition: {}", m_partitionDefinition);
}

void BistMarketAccessor::logout()
{
    protocol::soupbintcp::LogoutRequest logout_request {};
    logout_request.packetLength = htobe16(3);
    logout_request.packetType = PacketType::LogoutRequest;

    const auto& byte_array = toByteArray(logout_request);

    m_tcpClient.write(byte_array.data(), byte_array.size());
    LOG_INFO("<= Sent logout request for partition: {}", m_partitionDefinition);
}

void BistMarketAccessor::prepareEnterOrderBuffer()
{
    protocol::soupbintcp::UnsequencedData soupbin_header {};
    soupbin_header.packetType = PacketType::UnsequencedData;
    soupbin_header.packetLength
        = htobe16(sizeof(protocol::soupbintcp::UnsequencedData) + sizeof(protocol::ouch::EnterOrder) - sizeof(PacketLength));

    std::memcpy(&m_enterOrderByteArray, &soupbin_header, sizeof(protocol::soupbintcp::UnsequencedData));

    protocol::ouch::EnterOrder enter_order {};
    enter_order.type = protocol::ouch::MessageType::EnterOrder;
    enter_order.time_in_force = protocol::ouch::TimeInForce::Day;
    enter_order.open_close = protocol::ouch::OpenClose::DefaultForTheAccount;

    // this is equity enter order setup.
    for (int i = 0; i < m_exchangeInfo.size(); ++i) {
        enter_order.exchange_info[i] = m_exchangeInfo[i];
    }
    enter_order.display_quantity.val = be64toh(0);
    enter_order.client_category = protocol::ouch::ClientCategory::Client;
    enter_order.offhours = protocol::ouch::OffHours::NormalHours;
    std::memset(&enter_order.reserved, 0, enter_order.reserved.size());

    std::memcpy(
        (char*)&m_enterOrderByteArray + sizeof(protocol::soupbintcp::UnsequencedData), &enter_order, sizeof(protocol::ouch::EnterOrder));
}

void BistMarketAccessor::prepareReplaceOrderBuffer()
{
    protocol::soupbintcp::UnsequencedData soupbin_header {};
    soupbin_header.packetType = PacketType::UnsequencedData;
    soupbin_header.packetLength
        = htobe16(sizeof(protocol::soupbintcp::UnsequencedData) + sizeof(protocol::ouch::ReplaceOrder) - sizeof(PacketLength));

    std::memcpy(&m_replaceOrderByteArray, &soupbin_header, sizeof(protocol::soupbintcp::UnsequencedData));

    protocol::ouch::ReplaceOrder replace_order {};
    replace_order.type = protocol::ouch::MessageType::ReplaceOrder;
    replace_order.quantity.val = be64toh(0);  // we are not supposed to replace order qty most of the time.
    replace_order.open_close = protocol::ouch::OpenClose::DefaultForTheAccount;
    replace_order.display_quantity.val = be64toh(0);
    replace_order.client_category = protocol::ouch::ClientCategory::NoChange;

    std::memset(&replace_order.reserved, 0, replace_order.reserved.size());

    std::memcpy((char*)&m_replaceOrderByteArray + sizeof(protocol::soupbintcp::UnsequencedData),
        &replace_order,
        sizeof(protocol::ouch::ReplaceOrder));
}

void BistMarketAccessor::prepareCancelOrderBuffer()
{
    protocol::soupbintcp::UnsequencedData soupbin_header {};
    soupbin_header.packetType = PacketType::UnsequencedData;
    soupbin_header.packetLength
        = htobe16(sizeof(protocol::soupbintcp::UnsequencedData) + sizeof(protocol::ouch::CancelOrder) - sizeof(PacketLength));

    std::memcpy(&m_cancelOrderByteArray, &soupbin_header, sizeof(protocol::soupbintcp::UnsequencedData));

    protocol::ouch::CancelOrder cancel_order {};
    cancel_order.type = protocol::ouch::MessageType::CancelOrder;

    std::memcpy(
        (char*)&m_cancelOrderByteArray + sizeof(protocol::soupbintcp::UnsequencedData), &cancel_order, sizeof(protocol::ouch::CancelOrder));
}

void BistMarketAccessor::sendEnterOrder(uint32_t orderbook_id, char side, uint32_t qty, int32_t price)
{
    const auto& token = m_orderManager.nextToken();

    auto* enter_order
        = reinterpret_cast<protocol::ouch::EnterOrder*>(m_enterOrderByteArray.data() + sizeof(protocol::soupbintcp::UnsequencedData));

    enter_order->order_token = token;
    enter_order->orderbook_id.val = be32toh(orderbook_id);
    enter_order->side = static_cast<Side>(side);
    enter_order->quantity.val = be64toh(qty);
    enter_order->price.val = be32toh(price);

    m_tcpClient.write(m_enterOrderByteArray.data(), m_enterOrderByteArray.size());
    LOG_TRACE_L3("<= Sent enter order: {}. Partititon: {}", *enter_order, m_partitionDefinition);
}

void BistMarketAccessor::sendReplaceOrder(const std::array<char, 14>& original_order_token, int32_t price)
{
    const auto& replacement_token = m_orderManager.nextToken();

    m_orderManager.pendingReplace(original_order_token, replacement_token);

    auto* replace_order
        = reinterpret_cast<protocol::ouch::ReplaceOrder*>(m_replaceOrderByteArray.data() + sizeof(protocol::soupbintcp::UnsequencedData));
    replace_order->existing_order_token = original_order_token;
    replace_order->replacement_order_token = replacement_token;
    replace_order->price.val = be32toh(price);

    m_tcpClient.write(m_replaceOrderByteArray.data(), m_replaceOrderByteArray.size());
    LOG_TRACE_L3("<= Sent replace order: {}. Partititon: {}", *replace_order, m_partitionDefinition);
}

void BistMarketAccessor::sendCancelOrder(const std::array<char, 14>& original_order_token)
{
    auto* cancel_order
        = reinterpret_cast<protocol::ouch::CancelOrder*>(m_cancelOrderByteArray.data() + sizeof(protocol::soupbintcp::UnsequencedData));
    cancel_order->order_token = original_order_token;

    m_tcpClient.write(m_cancelOrderByteArray.data(), m_cancelOrderByteArray.size());
    LOG_TRACE_L3("<= Sent cancel order: {}. Partititon: {}", *cancel_order, m_partitionDefinition);
}

void BistMarketAccessor::onLoginAccepted(protocol::soupbintcp::LoginAccepted& login_accepted)
{
    const auto prev_state = m_state;
    m_state = State::Continuous;
    LOG_INFO("Login accepted: {}. Partition: {}", login_accepted, m_partitionDefinition);
    LOG_INFO("BIST market accessor state: {} => {}", magic_enum::enum_name(prev_state), magic_enum::enum_name(m_state));

    m_sessionName = std::string(login_accepted.session_name.begin(), login_accepted.session_name.end());
    LOG_INFO("Session name set as {} for partititon {}", m_sessionName, m_partitionDefinition);

    // if (m_partition.m_instrumentType == OrderEntryPartitionConfig::InstrumentType::Equity) {
    //     EnterOrder enter_order {};
    //     const auto& token = m_orderManager.nextToken();

    //     enter_order.type = ouch::MessageType::EnterOrder;  // I had forgotten this, this should fix.
    //     enter_order.order_token = token;
    //     enter_order.orderbook_id.val = 78'436;  // THYAO.E
    //     enter_order.side = Side::Sell;
    //     enter_order.quantity.val = 10;
    //     enter_order.price.val = 550'000;
    //     enter_order.time_in_force = TimeInForce::Day;
    //     enter_order.open_close = OpenClose::Default;
    //     // for (int i = 0; i < m_clientAccount.size() && i < m_clientAccount.size(); ++i) {
    //     //     enter_order.client_account[i] = m_clientAccount[i];
    //     // }
    //     // IMPORTANT NOTE: FOR "CLIENT" CLIENT CATEGORY, CLIENT ACCOUNT WILL NOT BE SET.

    //     for (int i = 0; i < m_exchangeInfo.size(); ++i) {
    //         enter_order.exchange_info[i] = m_exchangeInfo[i];
    //     }
    //     enter_order.display_quantity.val = 0;
    //     enter_order.client_category = ClientCategory::Client;
    //     enter_order.offhours = OffHours::NormalHours;

    //     std::memset(&enter_order.reserved, 0, 7);  // this is probably not needed.

    //     sendEnterOrder(enter_order);
    // }

    if (m_partition.m_instrumentType == OrderEntryPartitionConfig::InstrumentType::Equity) {
        // THYAO.E
        // for testing.
        sendEnterOrder(78'436, static_cast<char>(Side::Sell), 5000, 770'000);
    }
}

void BistMarketAccessor::onLoginRejected(protocol::soupbintcp::LoginRejected& login_rejected)
{
    const auto prev_state = m_state;
    m_state = State::Disconnected;
    LOG_ERROR("Login rejected: {}. Partition: {}", login_rejected, m_partitionDefinition);
    LOG_ERROR("BIST market accessor state: {} => {}", magic_enum::enum_name(prev_state), magic_enum::enum_name(m_state));
}

}  // namespace algocor
