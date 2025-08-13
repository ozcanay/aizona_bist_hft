#pragma once

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) OrderRejected {
    MessageType type;
    TimestampOuchNanoseconds timestamp;
    OrderToken order_token;
    RejectCode reject_code;

    friend std::ostream& operator<<(std::ostream& os, OrderRejected const& order)
    {
        os << "{"
           << "  type: '" << magic_enum::enum_name(order.type) << "'"
           << ", timestamp: " << order.timestamp << ", order_token: \"" << std::string(order.order_token.data(), order.order_token.size())
           << "\""
           << ", reject_code: "
           << static_cast<int32_t>(
                  order.reject_code)  // TODO: magic enum did not work on reject code. because it does not work well with negative values.
           << " }\n";
        return os;
    }
};
static_assert(sizeof(OrderRejected) == 27);
static_assert(std::is_trivial_v<OrderRejected> && std::is_standard_layout_v<OrderRejected>);
}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::OrderRejected> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::OrderRejected> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::OrderRejected> {};
