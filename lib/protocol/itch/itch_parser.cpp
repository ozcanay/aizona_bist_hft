#include "itch_parser.hpp"

// #include "client/market_data_client.hpp"

#include "itch_add_order.hpp"
#include "itch_add_order_with_mpid.hpp"
#include "itch_combination_orderbook_leg.hpp"
#include "itch_equilibrium_price_update.hpp"
#include "itch_order_delete.hpp"
#include "itch_order_executed.hpp"
#include "itch_order_executed_with_price.hpp"
#include "itch_order_replace.hpp"
#include "itch_orderbook_directory.hpp"
#include "itch_orderbook_flush.hpp"
#include "itch_orderbook_state.hpp"
#include "itch_seconds.hpp"
#include "itch_short_sell_status.hpp"
#include "itch_system_event.hpp"
#include "itch_tick_size_table_entry.hpp"
#include "itch_trade.hpp"

#include "../moldudp64/moldudp64_downstream_header.hpp"
#include "../moldudp64/moldudp64_message_block.hpp"

namespace
{

[[nodiscard]] std::string toStringSession(const std::array<char, 10>& token)
{
    std::string str;
    str.resize(token.size());
    for (int i = 0; i < token.size(); ++i) {
        str[i] = token[i];
    }
    return str;
}

}  // namespace

namespace algocor::protocol::itch
{

// ItchParser::ItchParser(MarketDataClient& marketDataClient)
//     : m_marketDataClient(marketDataClient)
// {
// }

void ItchParser::handleOrderAdd(const AddOrder* order_add)
{
    const auto order_id = be64toh(order_add->order_id);
    const auto orderbook_id = be32toh(order_add->orderbook_id);
    const auto price = static_cast<std::int32_t>(be32toh(order_add->price));
    const auto qty = be64toh(order_add->quantity);

    ItchOrder order {};
    order.id = order_id;
    order.price = price;
    order.side = static_cast<char>(order_add->side);
    order.left_qty = qty;
    order.orderbook_id = orderbook_id;
    m_orderMap[{ orderbook_id, order_id, order.side }] = order;

    auto& orderbook = m_orderbookMap[orderbook_id];
    orderbook.AddOrder(static_cast<char>(order_add->side), price, qty);

    orderbook.Dump();

    // m_orderMap[order.id] = order;
}

void ItchParser::handleOrderDelete(const OrderDelete* order_delete)
{
    const auto order_id = be64toh(order_delete->order_id);
    const auto orderbook_id = be32toh(order_delete->orderbook_id);
    if (!m_orderMap.contains({ orderbook_id, order_id, static_cast<char>(order_delete->side) })) {
        LOG_ERROR("Order with ID {} could not be found in map while attempting to delete the order", order_id);
        return;
    }

    auto& orderbook = m_orderbookMap[be32toh(order_delete->orderbook_id)];
    orderbook.DeleteOrder(static_cast<char>(order_delete->side),
        m_orderMap[{ orderbook_id, order_id, static_cast<char>(order_delete->side) }].price,
        m_orderMap[{ orderbook_id, order_id, static_cast<char>(order_delete->side) }].left_qty);

    m_orderMap.erase({ orderbook_id, order_id, static_cast<char>(order_delete->side) });

    orderbook.Dump();
}

void ItchParser::handleOrderExecution(const OrderExecuted* order_executed)
{
    const auto order_id = be64toh(order_executed->order_id);
    const auto orderbook_id = be32toh(order_executed->orderbook_id);
    if (!m_orderMap.contains({ orderbook_id, order_id, static_cast<char>(order_executed->side) })) {
        LOG_ERROR("Order with ID {} could not be found in map while attempting to execute the order", order_id);
        return;
    }

    auto& orderbook = m_orderbookMap[be32toh(order_executed->orderbook_id)];
    const auto executed_qty = be64toh(order_executed->quantity);
    orderbook.ExecuteOrder(static_cast<char>(order_executed->side),
        m_orderMap[{ orderbook_id, order_id, static_cast<char>(order_executed->side) }].price,
        executed_qty);

    m_orderMap[{ orderbook_id, order_id, static_cast<char>(order_executed->side) }].left_qty -= executed_qty;
    if (m_orderMap[{ orderbook_id, order_id, static_cast<char>(order_executed->side) }].left_qty == 0) {
        m_orderMap.erase({ orderbook_id, order_id, static_cast<char>(order_executed->side) });
    }

    orderbook.Dump();
}

std::pair<uint64_t, uint16_t> ItchParser::parse(const char* byte_array, size_t size)
{
    LOG_TRACE_L3("Parsing market data of size {}", size);
    const moldudp64::DownstreamHeader* packet_header = (moldudp64::DownstreamHeader*)(byte_array);  // TODO: use std::bit_cast
    if (packet_header == nullptr) {
        LOG_ERROR("Packet header is null");
        return {};
    }

    LOG_TRACE_L3("packet_header->session: {}", toStringSession(packet_header->session));
    // m_marketDataClient.setSessionName(packet_header->session);

    const auto message_count = be16toh(packet_header->message_count);
    const auto sequence_number = be64toh(packet_header->sequence_number);
    // LOG_TRACE_L3("sequence_number: {}, message_count: {}. Partition name: {}",
    //     sequence_number,
    //     message_count,
    //     m_marketDataClient.getPartitionConfig().name);

    parsePayload((const char*)packet_header + sizeof(moldudp64::DownstreamHeader), message_count, sequence_number);

    return { sequence_number, message_count };
}

void ItchParser::parsePayload(const char* payload, uint16_t message_count, uint64_t sequence_number)
{
    LOG_TRACE_L3("Parsing ITCH message. Message count: {}", message_count);
    uint32_t message_offset = 0;
    for (uint32_t message = 0; message < message_count; ++message) {
        const auto* message_block = (const struct moldudp64::MessageBlock*)(payload + message_offset);
        message_offset += be16toh(message_block->length) + sizeof(message_block->length);
        const auto currentSeqNo = sequence_number + message;

        if (message_block->data[0] == static_cast<char>(MessageType::AddOrder)) {
            auto* mb = (struct AddOrder*)message_block->data;
            LOG_TRACE_L3("#{} Add Order: {}", currentSeqNo, *mb);
            handleOrderAdd(mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::OrderExecuted)) {
            auto* mb = (struct OrderExecuted*)message_block->data;
            LOG_TRACE_L3("#{} Order executed: {}", currentSeqNo, *mb);
            handleOrderExecution(mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::OrderDelete)) {
            auto* mb = (struct OrderDelete*)message_block->data;
            LOG_TRACE_L3("#{} Order deleted: {}", currentSeqNo, *mb);
            handleOrderDelete(mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::OrderbookState)) {
            const struct OrderBookState* mb = (struct OrderBookState*)message_block->data;
            LOG_TRACE_L3("#{} Orderbook state: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::EquilibriumPriceUpdate)) {
            const struct EquilibriumPriceUpdate* mb = (struct EquilibriumPriceUpdate*)message_block->data;
            LOG_TRACE_L3("#{} Equilibrium state: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::OrderbookDirectory)) {
            const struct OrderBookDirectory* mb = (struct OrderBookDirectory*)message_block->data;
            LOG_TRACE_L3("#{} Orderbook directory: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::TickSizeTableEntry)) {
            const struct TickSizeTableEntry* mb = (struct TickSizeTableEntry*)message_block->data;
            LOG_TRACE_L3("#{} Tick size table entry: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::Trade)) {
            const struct Trade* mb = (struct Trade*)message_block->data;
            LOG_TRACE_L3("#{} Trade: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::OrderReplace)) {
            LOG_ERROR("BIST does not utilize order replace messages on ITCH protocol, but we did receive order replace! Discarding it.");
        } else if (message_block->data[0] == static_cast<char>(MessageType::Seconds)) {
            const struct Seconds* mb = (struct Seconds*)message_block->data;
            LOG_TRACE_L3("#{} ITCH seconds message: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::SystemEvent)) {
            const struct SystemEvent* mb = (struct SystemEvent*)message_block->data;
            LOG_TRACE_L3("#{} System event: {}", currentSeqNo, *mb);
        } else if (message_block->data[0] == static_cast<char>(MessageType::ShortSellStatus)) {
            LOG_WARNING("#{} Short sell parsing not implemented yet", currentSeqNo);
        } else {
            LOG_ERROR("#{} Unhandled message type ({}) received from ITCH", currentSeqNo, message_block->data[0]);
        }
    }
    LOG_TRACE_L3("Parsed {} ITCH messages.", message_count);
}

};  // namespace algocor::protocol::itch
