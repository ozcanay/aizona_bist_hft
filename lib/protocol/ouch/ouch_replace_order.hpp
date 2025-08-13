#pragma once

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) ReplaceOrder {
    MessageType type;                 // "U" mapping
    OrderToken existing_order_token;  // Should be the Order Token from the original Enter
    OrderToken replacement_order_token;
    Quantity quantity;               // Desired Open Quantity of the order
    Price price;                     // Setting Price to 0 means “no change”.
    OpenClose open_close;            // 0 = No change, 1 = Open, 2 = Close/Net, 4 = Default for the account
    ClientAccount client_account;    // Pass-thru field
    CustomerInfo customer_info;      // Pass-thru field
    ExchangeInfo exchange_info;      // Client account number, Only the first 16 bytes are used.
    Quantity display_quantity;       // Desired displayed quantity (zero for unchanged).
    ClientCategory client_category;  // 1 = Client, 2 = House, 7 = Fund, 9 = Investment Trust,
                                     // 10 = Primary Dealer Govt, 11 = Primary Dealer Corp, 12 = Portfolio Mgmt Company
    std::array<char, 8> reserved;

    friend std::ostream& operator<<(std::ostream& os, ReplaceOrder const& order)
    {
        os << "{"
           << "  type: \"" << magic_enum::enum_name(order.type) << "\""
           << ", existing_order_token: \"" << std::string(order.existing_order_token.data(), order.existing_order_token.size()) << "\""
           << ", replacement_order_token: \"" << std::string(order.replacement_order_token.data(), order.replacement_order_token.size())
           << "\""
           << ", quantity: " << be64toh(order.quantity) << ", price: " << be32toh(order.price)
           << ", open_close: " << static_cast<int>(order.open_close) << ", client_account: \""
           << std::string(order.client_account.data(), order.client_account.size()) << "\""
           << ", customer_info: \"" << std::string(order.customer_info.data(), order.customer_info.size()) << "\""
           << ", exchange_info: \"" << std::string(order.exchange_info.data(), order.exchange_info.size()) << "\""
           << ", display_quantity: " << be64toh(order.display_quantity) << ", client_category: " << static_cast<int>(order.client_category)
           << " }\n";
        return os;
    }
};
static_assert(sizeof(ReplaceOrder) == 122);
static_assert(std::is_trivial_v<ReplaceOrder> && std::is_standard_layout_v<ReplaceOrder>);

}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::ReplaceOrder> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::ReplaceOrder> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::ReplaceOrder> {};
