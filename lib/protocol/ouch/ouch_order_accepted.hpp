#pragma once

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) OrderAccepted {
    MessageType type;
    TimestampOuchNanoseconds timestamp;
    OrderToken order_token;
    OrderbookId orderbook_id;
    Side side;
    OrderId order_id;
    Quantity quantity;
    Price price;
    TimeInForce time_in_force;
    OpenClose open_close;
    ClientAccount client_account;
    OrderState order_state;
    CustomerInfo customer_info;
    ExchangeInfo exchange_info;
    Quantity pre_trade_quantity;
    Quantity display_quantity;
    ClientCategory client_category;
    OffHours offhours;
    std::array<char, 3> reserved;

    friend std::ostream& operator<<(std::ostream& os, OrderAccepted const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", timestamp: " << be64toh(order.timestamp) << ", order_token: \""
           << std::string(order.order_token.data(), order.order_token.size()) << "\""
           << ", orderbook_id: " << be32toh(order.orderbook_id) << ", side: " << magic_enum::enum_name(order.side)
           << ", order_id: " << be64toh(order.order_id) << ", quantity: " << be64toh(order.quantity) << ", price: " << be32toh(order.price)
           << ", time_in_force: " << magic_enum::enum_name(order.time_in_force)
           << ", open_close: " << magic_enum::enum_name(order.open_close) << ", client_account: \""
           << std::string(order.client_account.data(), order.client_account.size()) << "\""
           << ", order_state: " << magic_enum::enum_name(order.order_state) << ", customer_info: \""
           << std::string(order.customer_info.data(), order.customer_info.size()) << "\""
           << ", exchange_info: \"" << std::string(order.exchange_info.data(), order.exchange_info.size()) << "\""
           << ", pre_trade_quantity: " << be64toh(order.pre_trade_quantity) << ", display_quantity: " << be64toh(order.display_quantity)
           << ", client_category: " << magic_enum::enum_name(order.client_category)
           << ", offhours: " << magic_enum::enum_name(order.offhours) << ", reserved: \""
           << std::string(order.reserved.data(), order.reserved.size()) << "\""
           << " }\n";
        return os;
    }
};
static_assert(sizeof(OrderAccepted) == 135);
static_assert(std::is_trivial_v<OrderAccepted> && std::is_standard_layout_v<OrderAccepted>);
}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::OrderAccepted> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::OrderAccepted> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::OrderAccepted> {};
