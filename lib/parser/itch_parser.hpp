#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>

#include "core/l2_orderbook.hpp"

namespace algocor
{
class MarketDataClient;
}

namespace algocor::protocol::itch
{

struct ItchOrder {
    uint64_t id;
    int32_t price;
    uint64_t left_qty;
    char side;
    uint32_t orderbook_id;
};

class ItchParser {
    class MarketDataClient& m_marketDataClient;
    std::unordered_map<uint32_t, L2Orderbook> m_orderbookMap;              // TODO: Optimize
    std::map<std::tuple<uint32_t, uint64_t, char>, ItchOrder> m_orderMap;  // TODO: Optimize

public:
    explicit ItchParser(class MarketDataClient& marketDataClient);

    void handleOrderAdd(const struct AddOrder* order_add);
    void handleOrderDelete(const struct OrderDelete* order_delete);
    void handleOrderExecution(const struct OrderExecuted* order_executed);
    // void handleOrderReplace(const OrderReplace* order_replace);
    [[nodiscard]] std::pair<uint64_t, uint16_t> parse(const char* byte_array, size_t size);
    void parsePayload(const char* payload, uint16_t message_count, uint64_t sequence_number);
};

}  // namespace algocor::protocol::itch
