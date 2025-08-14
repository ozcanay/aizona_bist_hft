#pragma once
#include "../core/orderbook_builder.hpp"
#include "../moldudp64/moldudp64_downstream_header.hpp"
#include "../moldudp64/moldudp64_message_block.hpp"
#include "itch_add_order.hpp"
#include "itch_order_delete.hpp"
#include "itch_order_executed.hpp"

#include <array>
#include <string>
#include <utility>

namespace algocor::protocol::itch
{

template<typename Builder>
class ItchParser {
    Builder* m_builder;
    std::string m_session;

public:
    explicit ItchParser(Builder& builder)
        : m_builder(&builder)
    {
    }

    std::pair<uint64_t, uint16_t> parse(const char* byte_array, size_t size)
    {
        LOG_TRACE_L3("Parsing market data of size {}", size);

        const auto* header = reinterpret_cast<const moldudp64::DownstreamHeader*>(byte_array);
        if (!header)
            return {};

        m_session = toStringSession(header->session);

        parsePayload(byte_array + sizeof(*header), be16toh(header->message_count), be64toh(header->sequence_number));

        return { be64toh(header->sequence_number), be16toh(header->message_count) };
    }

    const std::string& getSession() const
    {
        return m_session;
    }

private:
    void handleOrderAdd(const AddOrder* order_add)
    {
        if (m_builder)
            m_builder->addOrder(*order_add);
    }

    void handleOrderExecution(const OrderExecuted* order_executed)
    {
        if (m_builder)
            m_builder->executeOrder(*order_executed);
    }

    void handleOrderDelete(const OrderDelete* order_delete)
    {
        if (m_builder)
            m_builder->deleteOrder(*order_delete);
    }

    void parsePayload(const char* payload, uint16_t message_count, uint64_t sequence_number)
    {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < message_count; ++i) {
            const auto* block = reinterpret_cast<const moldudp64::MessageBlock*>(payload + offset);
            offset += be16toh(block->length) + sizeof(block->length);

            switch (static_cast<MessageType>(block->data[0])) {
                case MessageType::AddOrder:
                    handleOrderAdd(reinterpret_cast<const AddOrder*>(block->data));
                    break;
                case MessageType::OrderExecuted:
                    handleOrderExecution(reinterpret_cast<const OrderExecuted*>(block->data));
                    break;
                case MessageType::OrderDelete:
                    handleOrderDelete(reinterpret_cast<const OrderDelete*>(block->data));
                    break;
                default:
                    break;  // other types ignored
            }
        }
    }

    // Helper
    static std::string toStringSession(const std::array<char, 10>& token)
    {
        return std::string(token.begin(), token.end());
    }
};

}  // namespace algocor::protocol::itch
