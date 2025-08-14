#pragma once

#include "../../constants.hpp"
#include "../../types.hpp"

#include <array>
#include <cstddef>
#include <fmt/base.h>
#include <magic_enum.hpp>

#include "quill/bundled/fmt/ostream.h"
#include "quill/core/Codec.h"
#include "quill/TriviallyCopyableCodec.h"
#include <iomanip>

namespace algocor::protocol::itch
{

namespace constants
{

static inline constexpr int ORDERBOOK_DIRECTORY_MSG_ORDERBOOK_ID_OFFSET
    = algocor::TIMESTAMP_NANOSECONDS_OFFSET + sizeof(algocor::TimestampNanoseconds);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_SYMBOL_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);

static inline constexpr int SYMBOL_LENGTH = 32;
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_LONG_NAME_OFFSET = ORDERBOOK_DIRECTORY_MSG_SYMBOL_OFFSET + SYMBOL_LENGTH;

static inline constexpr int LONG_NAME_LENGTH = 32;
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_ISIN_OFFSET = ORDERBOOK_DIRECTORY_MSG_LONG_NAME_OFFSET + LONG_NAME_LENGTH;

static inline constexpr int ISIN_LENGTH = 12;
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_FINANCIAL_PRODUCT_OFFSET = ORDERBOOK_DIRECTORY_MSG_ISIN_OFFSET + ISIN_LENGTH;

static inline constexpr int ORDERBOOK_DIRECTORY_MSG_TRADING_CURRENCY_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_FINANCIAL_PRODUCT_OFFSET + sizeof(algocor::FinancialProduct);

static inline constexpr int TRADING_CURRENCY_LENGTH = 3;
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_NUM_DECIMALS_IN_PRICE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_TRADING_CURRENCY_OFFSET + TRADING_CURRENCY_LENGTH;

static inline constexpr int ORDERBOOK_DIRECTORY_MSG_NUM_DECIMALS_IN_NOMINAL_VAL_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_NUM_DECIMALS_IN_PRICE_OFFSET + sizeof(algocor::NumberOfDecimalsInPrice);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_ODD_LOT_SIZE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_NUM_DECIMALS_IN_NOMINAL_VAL_OFFSET + sizeof(algocor::NumberOfDecimalsInPrice);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_ROUND_LOT_SIZE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_ODD_LOT_SIZE_OFFSET + sizeof(algocor::LotSize);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_BLOCK_LOT_SIZE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_ROUND_LOT_SIZE_OFFSET + sizeof(algocor::LotSize);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_NOMINAL_VAL_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_BLOCK_LOT_SIZE_OFFSET + sizeof(algocor::LotSize);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_NUM_OF_LEGS_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_NOMINAL_VAL_OFFSET + sizeof(algocor::NominalValue);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_UNDERLYING_ORDERBOOK_ID_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_NUM_OF_LEGS_OFFSET + sizeof(algocor::NumberOfLegs);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_STRIKE_PRICE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_UNDERLYING_ORDERBOOK_ID_OFFSET + sizeof(algocor::OrderbookId);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_EXPIRATION_DATE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_STRIKE_PRICE_OFFSET + sizeof(algocor::Price);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_NUM_OF_DECIMALS_IN_STRIKE_PRICE_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_EXPIRATION_DATE_OFFSET + sizeof(algocor::ExpirationDate);
static inline constexpr int ORDERBOOK_DIRECTORY_MSG_PUT_OR_CALL_OFFSET
    = ORDERBOOK_DIRECTORY_MSG_NUM_OF_DECIMALS_IN_STRIKE_PRICE_OFFSET + sizeof(algocor::NumberOfDecimalsInPrice);

static inline constexpr size_t ORDERBOOK_DIRECTORY_MSG_SIZE = ORDERBOOK_DIRECTORY_MSG_PUT_OR_CALL_OFFSET + sizeof(algocor::PutOrCall);

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
static_assert(ORDERBOOK_DIRECTORY_MSG_ORDERBOOK_ID_OFFSET == 5);
static_assert(ORDERBOOK_DIRECTORY_MSG_SYMBOL_OFFSET == 9);
static_assert(ORDERBOOK_DIRECTORY_MSG_LONG_NAME_OFFSET == 41);
static_assert(ORDERBOOK_DIRECTORY_MSG_ISIN_OFFSET == 73);
static_assert(ORDERBOOK_DIRECTORY_MSG_FINANCIAL_PRODUCT_OFFSET == 85);
static_assert(ORDERBOOK_DIRECTORY_MSG_TRADING_CURRENCY_OFFSET == 86);
static_assert(ORDERBOOK_DIRECTORY_MSG_NUM_DECIMALS_IN_PRICE_OFFSET == 89);
static_assert(ORDERBOOK_DIRECTORY_MSG_NUM_DECIMALS_IN_NOMINAL_VAL_OFFSET == 91);
static_assert(ORDERBOOK_DIRECTORY_MSG_ODD_LOT_SIZE_OFFSET == 93);
static_assert(ORDERBOOK_DIRECTORY_MSG_ROUND_LOT_SIZE_OFFSET == 97);
static_assert(ORDERBOOK_DIRECTORY_MSG_BLOCK_LOT_SIZE_OFFSET == 101);
static_assert(ORDERBOOK_DIRECTORY_MSG_NOMINAL_VAL_OFFSET == 105);
static_assert(ORDERBOOK_DIRECTORY_MSG_NUM_OF_LEGS_OFFSET == 113);
static_assert(ORDERBOOK_DIRECTORY_MSG_UNDERLYING_ORDERBOOK_ID_OFFSET == 114);
static_assert(ORDERBOOK_DIRECTORY_MSG_STRIKE_PRICE_OFFSET == 118);
static_assert(ORDERBOOK_DIRECTORY_MSG_EXPIRATION_DATE_OFFSET == 122);
static_assert(ORDERBOOK_DIRECTORY_MSG_NUM_OF_DECIMALS_IN_STRIKE_PRICE_OFFSET == 126);
static_assert(ORDERBOOK_DIRECTORY_MSG_PUT_OR_CALL_OFFSET == 128);
static_assert(ORDERBOOK_DIRECTORY_MSG_SIZE == 129);
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}  // namespace constants

struct __attribute__((packed)) OrderBookDirectory {
    algocor::MessageType type;
    algocor::TimestampNanoseconds nanoseconds;
    algocor::OrderbookId orderbook_id;
    std::array<char, constants::SYMBOL_LENGTH> symbol;
    std::array<char, constants::LONG_NAME_LENGTH> long_name;
    std::array<char, constants::ISIN_LENGTH> isin;
    algocor::FinancialProduct financial_product;
    std::array<char, constants::TRADING_CURRENCY_LENGTH> trading_currency;
    algocor::NumberOfDecimalsInPrice num_decimals_in_price;
    algocor::NumberOfDecimalsInPrice num_decimals_in_nominal_price;
    algocor::LotSize odd_lot_size;
    algocor::LotSize round_lot_size;
    algocor::LotSize block_lot_size;
    algocor::NominalValue nominal_value;
    algocor::NumberOfLegs num_legs;
    algocor::OrderbookId underlying_orderbook_id;
    algocor::Price strike_price;
    algocor::ExpirationDate expiration_date;
    algocor::NumberOfDecimalsInPrice num_decimals_in_strike_price;
    algocor::PutOrCall put_or_call;

    friend std::ostream& operator<<(std::ostream& os, OrderBookDirectory const& directory)
    {
        os << "{"
           << "  type: " << magic_enum::enum_name(directory.type) << ", nanoseconds: " << std::setw(10) << be32toh(directory.nanoseconds)
           << ", orderbook_id: " << std::setw(10) << be32toh(directory.orderbook_id)
           << ", symbol: " << std::string(directory.symbol.data(), directory.symbol.size())
           << ", long_name: " << std::string(directory.long_name.data(), directory.long_name.size())
           << ", isin: " << std::string(directory.isin.data(), directory.isin.size())
           << ", financial_product: " << magic_enum::enum_name(directory.financial_product)
           << ", trading_currency: " << std::string(directory.trading_currency.data(), directory.trading_currency.size())
           << ", num_decimals_in_price: " << std::setw(2) << be16toh(directory.num_decimals_in_price)
           << ", num_decimals_in_nominal_price: " << std::setw(2) << be16toh(directory.num_decimals_in_nominal_price)
           << ", odd_lot_size: " << std::setw(6) << be32toh(directory.odd_lot_size) << ", round_lot_size: " << std::setw(6)
           << be32toh(directory.round_lot_size) << ", block_lot_size: " << std::setw(6) << be32toh(directory.block_lot_size)
           << ", nominal_value: " << std::setw(10) << be64toh(directory.nominal_value) << ", num_legs: " << std::setw(2)
           << directory.num_legs.val << ", underlying_orderbook_id: " << std::setw(10) << be32toh(directory.underlying_orderbook_id)
           << ", strike_price: " << std::setw(10) << be32toh(directory.strike_price) << ", expiration_date: " << std::setw(10)
           << be32toh(directory.expiration_date) << ", num_decimals_in_strike_price: " << std::setw(2)
           << be16toh(directory.num_decimals_in_strike_price) << ", put_or_call: " << magic_enum::enum_name(directory.put_or_call)
           << " }\n";
        return os;
    }
};
static_assert(constants::ORDERBOOK_DIRECTORY_MSG_SIZE == sizeof(OrderBookDirectory));
static_assert(std::is_trivial_v<OrderBookDirectory> && std::is_standard_layout_v<OrderBookDirectory>);

}  // namespace algocor::protocol::itch

template<>
struct fmtquill::formatter<algocor::protocol::itch::OrderBookDirectory> : fmtquill::ostream_formatter {};

template<>
struct quill::Codec<algocor::protocol::itch::OrderBookDirectory>
    : quill::TriviallyCopyableTypeCodec<algocor::protocol::itch::OrderBookDirectory> {};
