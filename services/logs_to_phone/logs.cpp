#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include <arpa/inet.h>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using json = nlohmann::json;

#pragma pack(push, 1)
struct SensorData {
    int64_t timestamp_unix;
    double temperature_DHT22;
    double temperature_DS18B20;
    double humidity;
    double water_level;
    double soil_moisture;
    double light_intensity;
};
#pragma pack(pop)

// Реализация 64-битного преобразования endianness
uint64_t htonll(uint64_t value) {
    static const int num = 42;
    if (*reinterpret_cast<const char*>(&num) == num) { // Little-endian
        return (static_cast<uint64_t>(htonl(value & 0xFFFFFFFF)) << 32) | htonl(value >> 32);
    } else { // Big-endian
        return value;
    }
}

uint64_t ntohll(uint64_t value) {
    return htonll(value);
}

const std::string DB_PATH = "/home/tovarichkek/services/data_server_farm/data.db";
const int TCP_PORT = 1488;
const std::string LOG_FILE = "/var/log/data_to_phone.log";

class Database {
    sqlite3* db;

public:
    Database() {
        if(sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }
    
    ~Database() {
        sqlite3_close(db);
    }

    std::vector<SensorData> get_data(int64_t unix_from, int64_t unix_to) {
        std::vector<SensorData> results;
        
        sqlite3_stmt* stmt;
        const char* sql = "SELECT timestamp_unix, temperature_DHT22, "
                          "temperature_DS18B20, humidity, water_level, "
                          "soil_moisture, light_intensity "
                          "FROM sensor_data "
                          "WHERE timestamp_unix BETWEEN ? AND ? "
                          "ORDER BY timestamp_unix;";

        if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_int64(stmt, 1, unix_from);
            sqlite3_bind_int64(stmt, 2, unix_to);

            while(sqlite3_step(stmt) == SQLITE_ROW) {
                SensorData data;
                data.timestamp_unix = sqlite3_column_int64(stmt, 0);
                data.temperature_DHT22 = sqlite3_column_double(stmt, 1);
                data.temperature_DS18B20 = sqlite3_column_double(stmt, 2);
                data.humidity = sqlite3_column_double(stmt, 3);
                data.water_level = sqlite3_column_double(stmt, 4);
                data.soil_moisture = sqlite3_column_double(stmt, 5);
                data.light_intensity = sqlite3_column_double(stmt, 6);
                
                results.push_back(data);
            }
            sqlite3_finalize(stmt);
        }
        return results;
    }
};

class Logger {
    std::ofstream log_file;
    
public:
    Logger() {
        log_file.open(LOG_FILE, std::ios::app);
        if(!log_file.is_open()) {
            throw std::runtime_error("Cannot open log file");
        }
    }

    void log(const std::string& ip, int64_t from, int64_t to, size_t count) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char time_str[20];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
        
        log_file << time_str 
                << " | IP: " << ip
                << " | From: " << from
                << " | To: " << to
                << " | Records sent: " << count
                << std::endl;
    }
};

void serialize_sensor_data(std::vector<char>& buffer, const SensorData& data) {
    // Преобразование полей в сетевой порядок
    uint64_t net_timestamp = htonll(static_cast<uint64_t>(data.timestamp_unix));
    uint64_t net_temp_dht, net_temp_ds, net_humidity, net_water, net_soil, net_light;
    
    memcpy(&net_temp_dht, &data.temperature_DHT22, sizeof(uint64_t));
    memcpy(&net_temp_ds, &data.temperature_DS18B20, sizeof(uint64_t));
    memcpy(&net_humidity, &data.humidity, sizeof(uint64_t));
    memcpy(&net_water, &data.water_level, sizeof(uint64_t));
    memcpy(&net_soil, &data.soil_moisture, sizeof(uint64_t));
    memcpy(&net_light, &data.light_intensity, sizeof(uint64_t));
    
    net_temp_dht = htonll(net_temp_dht);
    net_temp_ds = htonll(net_temp_ds);
    net_humidity = htonll(net_humidity);
    net_water = htonll(net_water);
    net_soil = htonll(net_soil);
    net_light = htonll(net_light);

    // Запись в буфер
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_timestamp), 
                  reinterpret_cast<char*>(&net_timestamp) + sizeof(net_timestamp));
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_temp_dht), 
                  reinterpret_cast<char*>(&net_temp_dht) + sizeof(net_temp_dht));
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_temp_ds), 
                  reinterpret_cast<char*>(&net_temp_ds) + sizeof(net_temp_ds));
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_humidity), 
                  reinterpret_cast<char*>(&net_humidity) + sizeof(net_humidity));
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_water), 
                  reinterpret_cast<char*>(&net_water) + sizeof(net_water));
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_soil), 
                  reinterpret_cast<char*>(&net_soil) + sizeof(net_soil));
    buffer.insert(buffer.end(), reinterpret_cast<char*>(&net_light), 
                  reinterpret_cast<char*>(&net_light) + sizeof(net_light));
}

void send_binary_data(tcp::socket& socket, const std::vector<SensorData>& data) {
    // Отправка количества записей
    uint32_t count = htonl(static_cast<uint32_t>(data.size()));
    asio::write(socket, asio::buffer(&count, sizeof(count)));

    // Сериализация и отправка данных
    std::vector<char> buffer;
    for(const auto& item : data) {
        serialize_sensor_data(buffer, item);
    }
    if(!buffer.empty()) {
        asio::write(socket, asio::buffer(buffer));
    }
}

void handle_client(tcp::socket socket, Database& db, Logger& logger) {
    try {
        std::string client_ip = socket.remote_endpoint().address().to_string();
        
        asio::streambuf buf;
        asio::read_until(socket, buf, '\n');
        
        std::istream is(&buf);
        std::string request_str;
        std::getline(is, request_str);
        
        auto request = json::parse(request_str);
        if(!request.contains("unix_time_from") || !request.contains("unix_time_to")) {
            throw std::runtime_error("Invalid request format");
        }
        
        int64_t unix_from = request["unix_time_from"];
        int64_t unix_to = request["unix_time_to"];
        
        if(unix_from > unix_to) {
            throw std::runtime_error("Invalid time range");
        }

        auto data = db.get_data(unix_from, unix_to);
        send_binary_data(socket, data);
        
        logger.log(client_ip, unix_from, unix_to, data.size());
        std::cout << "Sent " << data.size() 
                 << " records to " << client_ip << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        std::ofstream tmp(LOG_FILE, std::ios::app);
        tmp.close();

        Database database;
        Logger logger;
        
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), TCP_PORT));

        std::cout << "Data to Phone Service started on port " << TCP_PORT << std::endl;

        while(true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            
            std::thread([s = std::move(socket), &database, &logger]() mutable {
                handle_client(std::move(s), database, logger);
            }).detach();
        }
    }
    catch(const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
