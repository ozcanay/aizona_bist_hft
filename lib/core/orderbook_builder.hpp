#pragma once
#include "../core/l2_orderbook.hpp"
#include "../protocol/itch/itch_add_order.hpp"
#include "../protocol/itch/itch_order_delete.hpp"
#include "../protocol/itch/itch_order_executed.hpp"
#include <cstdint>
#include <map>
#include <tuple>
#include <unordered_map>

namespace algocor::protocol::itch
{

struct ItchOrder {
    uint64_t id;
    int32_t price;
    uint64_t left_qty;
    char side;
    uint32_t orderbook_id;
};

// CRTP static-polymorphism builder
template<typename Derived>
class OrderbookBuilder {
public:
    std::unordered_map<uint32_t, L2Orderbook> m_orderbookMap;
    std::map<std::tuple<uint32_t, uint64_t, char>, ItchOrder> m_orderMap;

public:
    void addOrder(const AddOrder& order)
    {
        static_cast<Derived*>(this)->addOrder(order);
    }
    void executeOrder(const OrderExecuted& order)
    {
        static_cast<Derived*>(this)->executeOrder(order);
    }
    void deleteOrder(const OrderDelete& order)
    {
        static_cast<Derived*>(this)->deleteOrder(order);
    }

    // Accessors for unit tests
    bool hasOrder(uint32_t orderbook_id, uint64_t order_id, char side) const
    {
        return m_orderMap.contains({ orderbook_id, order_id, side });
    }

    const ItchOrder& getOrder(uint32_t orderbook_id, uint64_t order_id, char side) const
    {
        return m_orderMap.at({ orderbook_id, order_id, side });
    }

    const L2Orderbook& getOrderbook(uint32_t orderbook_id) const
    {
        return m_orderbookMap.at(orderbook_id);
    }
};

class ConcreteOrderbookBuilder : public OrderbookBuilder<ConcreteOrderbookBuilder> {
public:
    void addOrder(const AddOrder& order_add)
    {
        const auto order_id = be64toh(order_add.order_id);
        const auto orderbook_id = be32toh(order_add.orderbook_id);
        const auto price = static_cast<int32_t>(be32toh(order_add.price));
        const auto qty = be64toh(order_add.quantity);
        const auto side = static_cast<char>(order_add.side);

        ItchOrder order { order_id, price, qty, side, orderbook_id };
        m_orderMap[{ orderbook_id, order_id, side }] = order;

        auto& orderbook = m_orderbookMap[orderbook_id];
        orderbook.AddOrder(side, price, qty);
    }

    void executeOrder(const OrderExecuted& order_executed)
    {
        const auto order_id = be64toh(order_executed.order_id);
        const auto orderbook_id = be32toh(order_executed.orderbook_id);
        const auto side = static_cast<char>(order_executed.side);
        const auto executed_qty = be64toh(order_executed.quantity);

        auto it = m_orderMap.find({ orderbook_id, order_id, side });
        if (it == m_orderMap.end())
            return;

        auto& orderbook = m_orderbookMap[orderbook_id];
        orderbook.ExecuteOrder(side, it->second.price, executed_qty);

        it->second.left_qty -= executed_qty;
        if (it->second.left_qty == 0)
            m_orderMap.erase(it);
    }

    void deleteOrder(const OrderDelete& order_delete)
    {
        const auto order_id = be64toh(order_delete.order_id);
        const auto orderbook_id = be32toh(order_delete.orderbook_id);
        const auto side = static_cast<char>(order_delete.side);

        auto it = m_orderMap.find({ orderbook_id, order_id, side });
        if (it == m_orderMap.end())
            return;

        auto& orderbook = m_orderbookMap[orderbook_id];
        orderbook.DeleteOrder(side, it->second.price, it->second.left_qty);

        m_orderMap.erase(it);
    }
};

}  // namespace algocor::protocol::itch
