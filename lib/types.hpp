#pragma once

#include <array>
#include <cstdint>
// #include <fmt/base.h>

#include "strong_type.hpp"

namespace algocor
{

// Both MoldUdp64 and SoupBinTcp uses this.
using SessionName = std::array<char, 10>;

static inline constexpr int ORDER_TOKEN_LENGTH = 14;
using OrderToken = std::array<char, ORDER_TOKEN_LENGTH>;

static inline constexpr int CLIENT_ACCOUNT_LENGTH = 16;
using ClientAccount = std::array<char, CLIENT_ACCOUNT_LENGTH>;

static inline constexpr int CUSTOMER_INFO_LENGTH = 15;
using CustomerInfo = std::array<char, CUSTOMER_INFO_LENGTH>;

static inline constexpr int EXCHANGE_INFO_LENGTH = 32;
using ExchangeInfo = std::array<char, EXCHANGE_INFO_LENGTH>;

// These are only for ITCH, OUCH has different mappings.
enum class MessageType : char
{
    AddOrder = 'A',
    AddOrderWithMPID = 'F',
    OrderDelete = 'D',
    OrderExecuted = 'E',
    OrderExecutedWithPrice = 'C',
    OrderReplace = 'U',
    Trade = 'P',
    EquilibriumPriceUpdate = 'Z',
    OrderbookFlush = 'Y',
    Seconds = 'T',
    OrderbookDirectory = 'R',
    CombinationOrderbookLeg = 'M',
    TickSizeTableEntry = 'L',
    ShortSellStatus = 'V',
    SystemEvent = 'S',
    OrderbookState = 'O',
};

enum class ProductType : uint32_t
{
    UNKNOWN_PRODUCT = 0,
    OPTION = 1,
    FORWARD = 2,
    FUTURE = 3,
    FRA = 4,
    CASH = 5,
    PAYMENT = 6,
    EXCHANGE_RATE = 7,
    INTEREST_RATE_SWAP = 8,
    REPO = 9,
    SYNTHETIC_BOX_LEG_REFERENCE = 10,
    STANDARD_COMBINATION = 11,
    GUARANTEE = 12,
    OTC_GENRAL = 13,
    EQUITY_WARRANT = 14,
    SECURITY_LENDING = 15,
    CERTIFICATE = 18,
};

enum class TradingStatus : uint32_t
{
    TRADING_STATUS_UNKNOWN = 0,
    TRADING_STATUS_OPEN,
    TRADING_STATUS_OPEN_EXTENDED,
    TRADING_STATUS_CLOSE,
    TRADING_STATUS_QUOTE,
    TRADING_STATUS_QUOTE_OPEN,
    TRADING_STATUS_QUOTE_CLOSE,
    TRADING_STATUS_QUOTE_CIRCUIT_BREAKER,
    TRADING_STATUS_SUSPEND,
    TRADING_STATUS_LAST_TRADING_PHASE,
    TRADING_STATUS_HALTED,
    TRADING_STATUS_PRE_OPEN,
    TRADING_STATUS_PRE_CLOSE,
    TRADING_STATUS_POST_TRADING,
    TRADING_STATUS_OPENNING_OR_CLOSING,
};

enum class Side : char
{
    Buy = 'B',
    Sell = 'S',
};

enum class PutOrCall : uint8_t
{
    Call = 1,
    Put = 2,
};
static_assert(sizeof(PutOrCall) == 1 /* && std::is_unsigned_v<PutOrCall>*/);

enum class LegSide : char
{
    AsDefined = 'B',
    Opposite = 'C',
};

enum class LotType : uint8_t
{
    RoundLot = 2,
};

enum class ImbalanceDirection : uint32_t
{  // TODO: why 32
    UNDEFINED = 0,
    BUY = 1,
    SELL = 2,
    NONE = 3,
};

enum class FinancialProduct : uint8_t
{
    Option = 1,
    Forward = 2,
    Future = 3,
    FRA = 4,
    Cash = 5,
    Payment = 6,
    ExchangeRate = 7,
    InterestRateSwap = 8,
    REPO = 9,
    SyntheticBoxLeg_Reference = 10,
    StandardCombination = 11,
    Guarantee = 12,
    OTCGeneral = 13,
    EquityWarrant = 14,
    SecurityLending = 15,
    Certificate = 18,
};

enum class ShortSellRestriction : uint8_t
{
    Allowed = 1,
    Prohibited = 2,
};

enum class ShortSellValidation : uint8_t
{
    NoValidation = 0,
    PriceGreaterOrEqualToLTP = 1,
    NotAllowed = 2,
};

enum class EventCode : char
{
    StartOfMessages = 'O',
    EndOfMessages = 'C',
};

enum class OccurredAtCross : char
{
    Yes = 'Y',
    No = 'N',
};

enum class Printable : char
{
    Yes = 'Y',
    No = 'N',
};

// Note: this is not used by derivatives market.
enum class ClientCategory : uint8_t
{
    Client = 1,
    House = 2,
    Fund = 7,
    InvestmentTrust = 9,
    PrimaryDealerGovt = 10,
    PrimaryDealerCorp = 11,
    PortfolioMgmtCompany = 12,
};

enum class OffHours : uint8_t
{
    NormalHours = 0,
    OffHour = 1,
};

enum class OrderState : uint8_t
{
    OnBook = 1,
    NotOnBook = 2,
    Paused = 98,
};

// TODO
enum class CancelReason : uint8_t
{
    wqe,
    asd,
};

// TODO:
enum class RejectCode : int32_t
{
    blabla = -800105,
    asd,
};

using TimestampNanoseconds = StrongType<uint32_t, struct TimestampNanoseconds_>;
using TimestampOuchNanoseconds = StrongType<uint64_t, struct TimestampOuchNanoseconds_>;
using MatchId = StrongType<uint64_t, struct MatchId_>;

static inline constexpr int OUCH_MATCH_ID_LENGTH = 12;
using OuchMatchId = std::array<uint8_t, OUCH_MATCH_ID_LENGTH>;

using ComboGroupId = StrongType<uint32_t, struct ComboGroupId_>;
using Quantity = StrongType<uint64_t, struct Quantity_>;
using OrderbookId = StrongType<uint32_t, struct OrderbookId_>;
using OrderId = StrongType<uint64_t, struct OrderId_>;

using Price = StrongType<int32_t, struct Price_>;
static_assert(std::is_signed_v<decltype(Price::val)>);

using OrderAttributes = StrongType<uint16_t, struct OrderAttributes_>;
using NumberOfDecimalsInPrice = StrongType<uint16_t, struct NumberOfDecimalsInPrice_>;
using LotSize = StrongType<uint32_t, struct OddLotSize_>;
using NominalValue = StrongType<uint64_t, struct NominalValue_>;
using NumberOfLegs = StrongType<uint8_t, struct NumberOfLegs_>;
using ExpirationDate = StrongType<uint32_t, struct ExpirationDate_>;
using LegRatio = StrongType<uint32_t, struct LegRatio_>;
using TickSize = StrongType<uint64_t, struct TickSize_>;
using OrderbookPosition = StrongType<uint32_t, struct OrderbookPosition_>;
using Second = StrongType<uint32_t, struct Second_>;

using SequenceNumber = StrongType<uint64_t, struct SequenceNumber_>;
using MessageCount = StrongType<uint16_t, struct MessageCount_>;
using MessageLength = StrongType<uint16_t, struct MessageLength_>;
using Port = StrongType<uint16_t, struct Port_>;

// Below are for OUCH.
enum class TimeInForce : std::uint8_t
{
    Day,
    ImmediateOrCancel,
    FillOrKill
};

enum class OpenClose : std::uint8_t
{
    Default,
    Open,
    Close,
};

}  // namespace algocor

// template<typename T, typename Tag>
// class fmt::formatter<algocor::StrongType<T, Tag>> {
// public:
//     // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
//     [[nodiscard]] constexpr auto parse(format_parse_context& ctx)
//     {
//         return ctx.begin();
//     }

//     template<typename Context>
//     [[nodiscard]] auto format(const algocor::StrongType<T, Tag>& strong_type, Context& ctx) const
//     {
//         return fmt::format_to(ctx.out(), "{}", strong_type.val);
//     }
// };