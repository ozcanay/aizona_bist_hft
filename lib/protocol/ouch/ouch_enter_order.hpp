#pragma once

#include <array>
#include <type_traits>

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) EnterOrder {
    MessageType type;
    OrderToken order_token;
    OrderbookId orderbook_id;
    Side side;
    Quantity quantity;
    Price price;
    TimeInForce time_in_force;
    OpenClose open_close;
    ClientAccount client_account;
    CustomerInfo customer_info;
    ExchangeInfo exchange_info;
    Quantity display_quantity;
    ClientCategory client_category;
    OffHours offhours;
    std::array<char, 7> reserved;

    friend std::ostream& operator<<(std::ostream& os, EnterOrder const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", order_token: \""
           << std::string(order.order_token.data(), order.order_token.size()) << "\""
           << ", orderbook_id: " << be32toh(order.orderbook_id) << ", side: " << magic_enum::enum_name(order.side)
           << ", quantity: " << be64toh(order.quantity) << ", price: " << be32toh(order.price)
           << ", time_in_force: " << magic_enum::enum_name(order.time_in_force)
           << ", open_close: " << magic_enum::enum_name(order.open_close)
           << ", client_account: " << std::string(order.client_account.data(), order.client_account.size())
           << ", customer_info: " << std::string(order.customer_info.data(), order.customer_info.size())
           << ", exchange_info: " << std::string(order.exchange_info.data(), order.exchange_info.size())
           << ", display_quantity: " << be64toh(order.display_quantity)
           << ", client_category: " << magic_enum::enum_name(order.client_category)
           << ", offhours: " << magic_enum::enum_name(order.offhours) << ", reserved: \""
           << std::string(order.reserved.data(), order.reserved.size()) << "\""
           << " }\n";
        return os;
    }
};
static_assert(sizeof(EnterOrder) == 114);
static_assert(std::is_trivial_v<EnterOrder> && std::is_standard_layout_v<EnterOrder>);

}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::EnterOrder> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::EnterOrder> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::EnterOrder> {};
