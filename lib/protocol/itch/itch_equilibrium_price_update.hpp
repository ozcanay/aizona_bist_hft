#pragma once

#include <cstddef>

#include "../../constants.hpp"
#include "../../types.hpp"

#include <fmt/base.h>
#include <iomanip>
#include <magic_enum.hpp>

#include "quill/bundled/fmt/format.h"
#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"

namespace algocor::protocol::itch
{

namespace constants
{

static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_AVAILABLE_BID_QTY_EQUI_PX_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_AVAILABLE_ASK_QTY_EQUI_PX_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_AVAILABLE_BID_QTY_EQUI_PX_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_EQUI_PX_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_AVAILABLE_ASK_QTY_EQUI_PX_OFFSET + sizeof(algocor::Quantity);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_BID_PRICE_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_EQUI_PX_OFFSET + sizeof(algocor::Price);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_ASK_PRICE_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_BID_PRICE_OFFSET + sizeof(algocor::Price);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_BID_QTY_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_ASK_PRICE_OFFSET + sizeof(algocor::Price);
static inline constexpr int EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_ASK_QTY_OFFSET
    = EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_BID_QTY_OFFSET + sizeof(algocor::Quantity);

static inline constexpr size_t EQUILIBRIUM_PRICE_UPDATE_MSG_SIZE
    = EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_ASK_QTY_OFFSET + sizeof(algocor::Quantity);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_ORDERBOOK_ID_OFFSET == 5);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_AVAILABLE_BID_QTY_EQUI_PX_OFFSET == 9);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_AVAILABLE_ASK_QTY_EQUI_PX_OFFSET == 17);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_EQUI_PX_OFFSET == 25);

// TODO: We are not making use of below offsets, why?
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_BID_PRICE_OFFSET == 29);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_ASK_PRICE_OFFSET == 33);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_BID_QTY_OFFSET == 37);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_BEST_ASK_QTY_OFFSET == 45);
static_assert(EQUILIBRIUM_PRICE_UPDATE_MSG_SIZE == 53);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace constants

struct __attribute__((packed)) EquilibriumPriceUpdate {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId orderbook_id;
    algocor::Quantity available_bid_quantity;
    algocor::Quantity available_ask_quantity;
    algocor::Price equilibrium_price;
    algocor::Price best_bid_price;
    algocor::Price best_ask_price;
    algocor::Quantity best_bid_quantity;
    algocor::Quantity best_ask_quantity;

    friend std::ostream& operator<<(std::ostream& os, EquilibriumPriceUpdate const& update)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(update.type) << ", nanoseconds: " << std::setw(10) << be32toh(update.nanoseconds)
           << ", orderbook_id: " << std::setw(10) << be32toh(update.orderbook_id) << ", available_bid_quantity: " << std::setw(6)
           << be64toh(update.available_bid_quantity) << ", available_ask_quantity: " << std::setw(6)
           << be64toh(update.available_ask_quantity) << ", equilibrium_price: " << std::setw(10) << be32toh(update.equilibrium_price)
           << ", best_bid_price: " << std::setw(10) << be32toh(update.best_bid_price) << ", best_ask_price: " << std::setw(10)
           << be32toh(update.best_ask_price) << ", best_bid_quantity: " << std::setw(6) << be64toh(update.best_bid_quantity)
           << ", best_ask_quantity: " << std::setw(6) << be64toh(update.best_ask_quantity) << " }\n";
        return os;
    }
};
static_assert(constants::EQUILIBRIUM_PRICE_UPDATE_MSG_SIZE == sizeof(EquilibriumPriceUpdate));
static_assert(std::is_trivial_v<EquilibriumPriceUpdate> && std::is_standard_layout_v<EquilibriumPriceUpdate>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::EquilibriumPriceUpdate> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::EquilibriumPriceUpdate>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::EquilibriumPriceUpdate> {};
