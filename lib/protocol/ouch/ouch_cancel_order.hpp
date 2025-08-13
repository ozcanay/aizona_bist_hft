#pragma once

#include "../../types.hpp"
#include "ouch_types.hpp"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <magic_enum.hpp>

namespace algocor::protocol::ouch
{
struct __attribute__((packed)) CancelOrder {
    MessageType type;
    OrderToken order_token;

    friend std::ostream& operator<<(std::ostream& os, CancelOrder const& order)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(order.type) << ", order_token: \""
           << std::string(order.order_token.data(), order.order_token.size()) << "\""
           << " }\n";
        return os;
    }
};
static_assert(sizeof(CancelOrder) == 15);
static_assert(std::is_trivial_v<CancelOrder> && std::is_standard_layout_v<CancelOrder>);

}  // namespace algocor::protocol::ouch

template<>
struct fmtquill::formatter<algocor::protocol::ouch::CancelOrder> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::ouch::CancelOrder> : quill::TriviallyCopyableTypeCodec<algocor::protocol::ouch::CancelOrder> {};
