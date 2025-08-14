#include "../core/orderbook_builder.hpp"
#include "../moldudp64/moldudp64_downstream_header.hpp"
#include "itch_add_order.hpp"
#include "itch_parser.hpp"
#include <gtest/gtest.h>
#include <pcap.h>

using namespace algocor::protocol::itch;

// --- Mock for static polymorphism ---
struct MockOrderbookBuilder {
    int add_calls = 0;
    int exec_calls = 0;
    int del_calls = 0;

    void addOrder(const AddOrder&)
    {
        ++add_calls;
    }
    void executeOrder(const OrderExecuted&)
    {
        ++exec_calls;
    }
    void deleteOrder(const OrderDelete&)
    {
        ++del_calls;
    }
};

// --- Helper to load pcap ---
static std::vector<std::vector<uint8_t>> loadPcap(const std::string& filename)
{
    std::vector<std::vector<uint8_t>> packets;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcap = pcap_open_offline(filename.c_str(), errbuf);
    if (!pcap)
        throw std::runtime_error(errbuf);

    struct pcap_pkthdr* header;
    const u_char* data;
    int ret;
    while ((ret = pcap_next_ex(pcap, &header, &data)) >= 0) {
        if (ret == 0)
            continue;
        packets.emplace_back(data, data + header->caplen);
    }
    pcap_close(pcap);
    return packets;
}

// --- Test with real builder to validate parsed orders ---
TEST(ItchParserRealBuilderTest, ParseAddOrders)
{
    setup_quill("test_log.txt", quill::LogLevel::TraceL3);

    ConcreteOrderbookBuilder builder;
    ItchParser<ConcreteOrderbookBuilder> parser(builder);

    auto packets = loadPcap("dummy_add_order.pcap");
    ASSERT_EQ(packets.size(), 1);

    const size_t payload_offset = 14 + 20 + 8;
    const auto& raw_packet = packets[0];
    ASSERT_GE(raw_packet.size(), payload_offset);
    const char* payload = reinterpret_cast<const char*>(raw_packet.data() + payload_offset);
    size_t payload_size = raw_packet.size() - payload_offset;

    auto [seq, msg_count] = parser.parse(payload, payload_size);
    EXPECT_EQ(seq, 1);
    EXPECT_EQ(msg_count, 3);

    // Validate orders in builder
    struct ExpectedOrder {
        uint32_t orderbook_id;
        uint64_t order_id;
        char side;
        uint64_t qty;
        int32_t price;
    };

    std::vector<ExpectedOrder> expected_orders = {
        { 1, 1001, 'B', 100, 1000 },
        { 1, 1002, 'S', 200, 1010 },
        { 1, 1003, 'B', 150, 1020 },
    };

    for (const auto& eo : expected_orders) {
        EXPECT_TRUE(builder.hasOrder(eo.orderbook_id, eo.order_id, eo.side));
        const auto& order = builder.getOrder(eo.orderbook_id, eo.order_id, eo.side);
        EXPECT_EQ(order.left_qty, eo.qty);
        EXPECT_EQ(order.price, eo.price);
        EXPECT_EQ(order.side, eo.side);
        EXPECT_EQ(order.orderbook_id, eo.orderbook_id);
    }
}

// --- Test with mock builder to verify parser forwarding ---
TEST(ItchParserMockBuilderTest, ForwardMessagesToBuilder)
{
    MockOrderbookBuilder mock;
    ItchParser<MockOrderbookBuilder> parser(mock);

    auto packets = loadPcap("dummy_add_order.pcap");
    ASSERT_EQ(packets.size(), 1);

    const size_t payload_offset = 14 + 20 + 8;
    const auto& raw_packet = packets[0];
    ASSERT_GE(raw_packet.size(), payload_offset);
    const char* payload = reinterpret_cast<const char*>(raw_packet.data() + payload_offset);
    size_t payload_size = raw_packet.size() - payload_offset;

    parser.parse(payload, payload_size);

    EXPECT_EQ(mock.add_calls, 3);   // three add orders in pcap
    EXPECT_EQ(mock.exec_calls, 0);  // no executed orders in this pcap
    EXPECT_EQ(mock.del_calls, 0);   // no deletes
}
