#pragma once

#include <cstdint>
#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "overwrite_macros.hpp"

struct MarketDataPartitionConfig {
    std::string name;
    enum class InstrumentType
    {
        Equity,
        Derivative
    } m_instrumentType;
    std::string multicast_ip;
    uint16_t multicast_port;
    std::string multicast_interface_ip;
    std::string unicast_request_ip;
    uint16_t unicast_request_port;
    std::string unicast_destination_ip;
    uint16_t unicast_destination_port;
    int cpu;

    [[nodiscard]] std::string toString() const
    {
        return fmt::format("Name: {}, Type: {}, Multicast IP: {}, Multicast Port: {}, "
                           "Multicast Interface IP: {}, Unicast Request IP: {}, "
                           "Unicast Request Port: {}, Unicast Destination IP: {}, "
                           "Unicast Destination Port: {}, CPU: {}",
            name,
            m_instrumentType == InstrumentType::Equity ? "Equity" : "Derivative",
            multicast_ip,
            multicast_port,
            multicast_interface_ip,
            unicast_request_ip,
            unicast_request_port,
            unicast_destination_ip,
            unicast_destination_port,
            cpu);
    }
};

struct MarketDataConfig {
    std::vector<MarketDataPartitionConfig> partition_configs;
};

struct OrderEntryPartitionConfig {
    std::string name;
    enum class InstrumentType
    {
        Equity,
        Derivative
    } m_instrumentType;
    std::string ip;
    uint16_t port;
    std::string port_str;  // TODO: have a single port variable that is std::string.

    std::string username;
    std::string password;

    [[nodiscard]] std::string toString() const
    {
        return fmt::format("Name: {}, Type: {}, IP: {}, Port: {}, Username: {}, Password: {}",
            name,
            m_instrumentType == InstrumentType::Equity ? "Equity" : "Derivative",
            ip,
            port,
            username,
            password);
    }
};

struct OrderEntryConfig {
    std::string client_account;
    std::string exchange_info;
    std::vector<OrderEntryPartitionConfig> partition_configs;
};

struct AppParameters {
    uint32_t last_used_token_index;
};

class ConfigParser {
public:
    explicit ConfigParser(const std::string& file_name)
        : m_file_name(file_name)
    {
    }

    bool parseAppParameters()
    {
        std::ifstream file(m_file_name);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file: {}", m_file_name);
            return false;
        }

        LOG_INFO("Parsing config file: {}", m_file_name);

        nlohmann::json j;
        try {
            file >> j;
        } catch (const std::exception& e) {
            LOG_ERROR("JSON parsing error: {}", e.what());
            return false;
        }

        if (j.contains("last_used_order_token_index") && j["last_used_order_token_index"].is_number_integer()) {
            m_appParams.last_used_token_index = j["last_used_order_token_index"].get<uint32_t>();
            LOG_INFO("Loaded last_used_order_token_index: {}", m_appParams.last_used_token_index);
        } else {
            LOG_ERROR("Missing or invalid last_used_order_token_index in config file");
            return false;
        }

        return true;
    }

    bool parseConfig()
    {
        std::ifstream file(m_file_name);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file: {}", m_file_name);
            return false;
        }

        LOG_INFO("Parsing config file: {}", m_file_name);

        nlohmann::json j;
        try {
            file >> j;
        } catch (const std::exception& e) {
            LOG_ERROR("JSON parsing error: {}", e.what());
            return false;
        }

        if (!j.contains("market_data") || !j.contains("order_entry")) {
            LOG_ERROR("Missing required top-level fields in JSON");
            return false;
        }

        if (!parseInstrumentConfig(j["market_data"], MarketDataPartitionConfig::InstrumentType::Equity, "equity"))
            return false;
        if (!parseInstrumentConfig(j["market_data"], MarketDataPartitionConfig::InstrumentType::Derivative, "derivative"))
            return false;
        if (!parseOrderEntryConfig(j["order_entry"]))
            return false;

        return true;
    }

    void printConfig() const
    {
        for (const auto& partition : m_marketDataConfig.partition_configs) {
            LOG_INFO("Market Data Partition: {}, Type: {}, Multicast IP: {}, Port: {}, Interface IP: {}, "
                     "Unicast Request IP: {}, Request Port: {}, Destination IP: {}, Destination Port: {}, CPU: {}",
                partition.name,
                partition.m_instrumentType == MarketDataPartitionConfig::InstrumentType::Equity ? "Equity" : "Derivative",
                partition.multicast_ip,
                partition.multicast_port,
                partition.multicast_interface_ip,
                partition.unicast_request_ip,
                partition.unicast_request_port,
                partition.unicast_destination_ip,
                partition.unicast_destination_port,
                partition.cpu);
        }

        LOG_INFO("Client Account: {}", m_orderEntryConfig.client_account);
        LOG_INFO("Exchange Info: {}", m_orderEntryConfig.exchange_info);

        for (const auto& entry : m_orderEntryConfig.partition_configs) {
            LOG_INFO("Order Entry Partition: {}, Type: {}, IP: {}, Port: {}, Username: {}, Password: {}",
                entry.name,
                entry.m_instrumentType == OrderEntryPartitionConfig::InstrumentType::Equity ? "Equity" : "Derivative",
                entry.ip,
                entry.port,
                entry.username,
                entry.password);
        }
    }

    [[nodiscard]] std::vector<MarketDataPartitionConfig> getMarketDataPartitionConfigs() const
    {
        return m_marketDataConfig.partition_configs;
    }

    [[nodiscard]] OrderEntryConfig getOrderEntryConfig() const
    {
        return m_orderEntryConfig;
    }

    [[nodiscard]] AppParameters getAppParameters() const
    {
        return m_appParams;
    }

private:
    std::string m_file_name;
    MarketDataConfig m_marketDataConfig;
    OrderEntryConfig m_orderEntryConfig;
    AppParameters m_appParams;

    bool parseInstrumentConfig(const nlohmann::json& market_data, MarketDataPartitionConfig::InstrumentType type, const std::string& key)
    {
        if (!market_data.contains(key)) {
            LOG_ERROR("Missing market data key: {}", key);
            return false;
        }

        auto& instrument_json = market_data[key];
        if (!instrument_json.contains("partitions")) {
            LOG_ERROR("Missing partitions field for {}", key);
            return false;
        }

        for (const auto& partition : instrument_json["partitions"]) {
            if (partition.contains("is_active") && !partition["is_active"].get<bool>()) {
                LOG_INFO("Skipping inactive market data partition: {}", partition["name"].get<std::string>());
                continue;
            }

            LOG_INFO("Processing active market data partition: {}", partition["name"].get<std::string>());

            if (!partition.contains("name") || !partition.contains("multicast_port") || !partition.contains("unicast_request_port")
                || !partition.contains("unicast_destination_port") || !partition.contains("cpu")) {
                LOG_ERROR("Missing required partition fields in {}", key);
                return false;
            }

            MarketDataPartitionConfig config;
            config.name = partition["name"];
            config.m_instrumentType = type;
            config.multicast_ip = instrument_json["itch_multicast_ip"];
            config.multicast_interface_ip = instrument_json["itch_multicast_interface_ip"];
            config.unicast_request_ip = instrument_json["rewinder_unicast_request_ip"];
            config.unicast_destination_ip = instrument_json["rewinder_unicast_destination_ip"];
            config.multicast_port = partition["multicast_port"];
            config.unicast_request_port = partition["unicast_request_port"];
            config.unicast_destination_port = partition["unicast_destination_port"];
            config.cpu = partition["cpu"];

            m_marketDataConfig.partition_configs.push_back(config);
        }
        return true;
    }

    bool parseOrderEntryConfig(const nlohmann::json& order_entry)
    {
        if (!order_entry.contains("client_account") || !order_entry.contains("exchange_info")) {
            LOG_ERROR("Missing client_account or exchange_info in order_entry");
            return false;
        }

        m_orderEntryConfig.client_account = order_entry["client_account"];
        m_orderEntryConfig.exchange_info = order_entry["exchange_info"];

        for (const std::string& key : { "equity", "derivative" }) {
            if (!order_entry.contains(key)) {
                LOG_ERROR("Missing order entry key: {}", key);
                return false;
            }

            auto& entry_json = order_entry[key];

            if (entry_json.contains("is_active") && !entry_json["is_active"].get<bool>()) {
                LOG_INFO("Skipping inactive order entry partition: {}", entry_json["name"].get<std::string>());
                continue;
            }

            LOG_INFO("Processing active order entry partition: {}", entry_json["name"].get<std::string>());

            if (!entry_json.contains("name") || !entry_json.contains("ip") || !entry_json.contains("port")
                || !entry_json.contains("username") || !entry_json.contains("password")) {
                LOG_ERROR("Missing required order entry fields in {}", key);
                return false;
            }

            OrderEntryPartitionConfig config;
            config.name = entry_json["name"];
            config.m_instrumentType = (key == "equity") ? OrderEntryPartitionConfig::InstrumentType::Equity
                                                        : OrderEntryPartitionConfig::InstrumentType::Derivative;
            config.ip = entry_json["ip"];
            config.port = entry_json["port"];
            config.username = entry_json["username"];
            config.password = entry_json["password"];

            m_orderEntryConfig.partition_configs.push_back(config);
        }
        return true;
    }
};
