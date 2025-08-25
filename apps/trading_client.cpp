#include <iostream>

#include "itch_add_order.hpp"
#include "itch_add_order_with_mpid.hpp"

#include "order_entry_client.hpp"

#include "../../lib/utility/quill_wrapper.hpp"

std::atomic_flag stopRequested = ATOMIC_FLAG_INIT;

int main()
{
    setup_quill("trading_client.txt", quill::LogLevel::TraceL3);

    OrderEntryPartitionConfig config;
    config.name = "LOCAL_TEST";
    config.ip = "127.0.0.1";
    config.port = 6642;
    config.port_str = "6642";
    config.m_instrumentType = OrderEntryPartitionConfig::InstrumentType::Equity;
    config.username = "TEST";
    config.password = "TEST_PWD";

    std::string acc = "CLIACC";
    std::string exc_info = "EXCINFO";

    algocor::BistMarketAccessor market_accessor(config, acc, exc_info, 1);
    market_accessor.login();

    while (1)
        ;
}