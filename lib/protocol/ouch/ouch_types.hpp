#pragma once

#include <cstdint>

namespace algocor::protocol::ouch
{

enum class MessageType : char
{
    EnterOrder = 'O',
    ReplaceOrder = 'U',
    CancelOrder = 'X',
    CancelByOrderId = 'Y',
    MassQuote = 'Q',
    OrderAccepted = 'A',
    OrderRejected = 'J',
    OrderReplaced = 'U',
    OrderCanceled = 'C',
    OrderExecuted = 'E',
    MassQuoteAcknowledgement = 'K',
    MassQuoteRejection = 'R',
};

enum class OpenClose : std::uint8_t
{
    NoChange = 0,
    Open = 1,
    CloseNet = 2,
    DefaultForTheAccount = 4,
};

enum class ClientCategory : std::uint8_t
{
    NoChange = 0,
    Client = 1,
    House = 2,
    Fund = 7,
    InvestmentTrust = 9,
    PrimaryDealerGovt = 10,
    PrimaryDealerCorp = 11,
    PortfolioMgmtCompany = 12,
    Error = 13,
};

enum class TimeInForce : std::uint8_t
{
    Day = 0,
    ImmediateOrCancel = 3,
    FillOrKill = 4,
    ERROR = 255,  // does this really exist?
};

enum class OffHours : std::uint8_t
{
    OffHoursOrders = 1,
    NormalHours = 0,
};

enum class OrderState : std::uint8_t
{
    OnBook = 1,
    NotOnBook = 2,
    Paused = 98,
    OUCHOrderOwnershipLost = 99,
};

// TODO: add the missing parts.
enum class CancelReason : std::uint8_t
{
    CanceledByUser = 1,
    Trade = 3,
    Inactivate = 4,
    ReplacedByUser = 5,
    New = 6,
    ConvertedBySystem = 8,
    CanceledBySystem = 9,
    CanceledByProxy = 10,
    BaitRecalculated = 11,
    TriggeredBySystem = 12,
    RefreshedBySystem = 13,
    CanceledBySystemLimitChange = 15,
    LinkedLegCanceled = 17,
    LinkedLegModified = 18,
    Expired = 19,
    CanceledDueToISS = 20,
    InactivatedDueToISS = 21,
    InactivatedDueToPurge = 23,
    InactivatedDayOrder = 24,
    InactivatedDueToDeList = 25,
    InactivatedDueToExpiry = 26,
    InactivatedDueToOutsideLimits = 27,
    TransferOfOwnership = 28,
    NewInactive = 29,
    Reloaded = 30,
    ReloadedIntraday = 31,
    CanceledAfterAuction = 34,
    InactivatedDueToOutsidePriceLimits = 35,
    ActivatedDueToOutsideLimits = 36,
    TriggerOnSessionOrderTriggered = 37,
    UndisclosedQuantityOrderConverted = 39,
    InactivatedDueToOrderValue = 40,
    CanceledBySystemDeltaProtection = 41,
    CanceledBySystemQuantityProtection = 42,
    InternalCrossingDelete = 43,
    CanceledDueToParticipantBlockOnMarket = 44,
    InactivatedDueToParticipantBlockOnMarket = 45,
    Paused = 52,
    ActivatedPausedOrder = 53,
    LinkedLegActivated = 56,
    DeletedPTRMmisc = 115,
    DeletedPTRMuserlimitsauto = 116,
    DeletedPTRMuserlimitsmanual = 117,
    DeletedPTRMmarketlimits = 118,
    DeletedPTRMinvestorlimits = 119,
    DeletedPTRMmarginbreach = 120,
    DeletedPTRMparticipantsuspension = 121,
    DeletedPTRMmrasuspension = 122,
    DeletedPTRMmcasuspension = 123,
    DeletedPTRMtasuspension = 124,
};

enum class QUOTESTATUS : std::uint32_t
{
    Accepted = 0,
    Updated = 1,
    Canceled = 2,
    UnsolicitedUpdate = 3,
    UnsolicitedCancel = 4,
    Traded = 5,
};

}  // namespace algocor::protocol::ouch
