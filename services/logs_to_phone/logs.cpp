#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <thread>
#include <boost/asio.hpp>
#include <sqlite3.h>
#include <nlohmann/json.hpp>

namespace asio = boost::asio;
using boost::asio::ip::tcp;
using json = nlohmann::json;

const std::string DB_PATH = "/home/tovarichkek/services/data_server_farm/data.db";
const int TCP_PORT = 1489;
const std::string LOG_FILE = "/var/log/data_to_phone.log";

class Database {
    sqlite3* db;
    
    std::tm get_target_date(int day_number) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm tm_current = *std::localtime(&now_time);

        // Пробуем текущий месяц
        std::tm tm_target = tm_current;
        tm_target.tm_mday = day_number;
        tm_target.tm_isdst = -1;
        std::mktime(&tm_target);

        // Проверяем валидность даты
        bool valid_current_month = 
            (tm_target.tm_mon == tm_current.tm_mon) &&
            (tm_target.tm_year == tm_current.tm_year) &&
            (now_time >= std::mktime(&tm_target));

        if(!valid_current_month) {
            // Пробуем предыдущий месяц
            tm_target = tm_current;
            tm_target.tm_mon--;
            tm_target.tm_mday = day_number;
            tm_target.tm_isdst = -1;
            std::mktime(&tm_target);

            // Если день не существует, берем последний день месяца
            if(tm_target.tm_mday != day_number) {
                tm_target.tm_mday = 0;
                std::mktime(&tm_target);
            }
        }

        return tm_target;
    }

public:
    Database() {
        if(sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error(sqlite3_errmsg(db));
        }
    }
    
    ~Database() {
        sqlite3_close(db);
    }

    json get_day_data(int day_number) {
        std::tm tm_target = get_target_date(day_number);

        // Начало дня
        std::tm tm_start = tm_target;
        tm_start.tm_hour = 0;
        tm_start.tm_min = 0;
        tm_start.tm_sec = 0;
        std::time_t start_time = std::mktime(&tm_start);

        // Конец дня
        std::tm tm_end = tm_target;
        tm_end.tm_hour = 23;
        tm_end.tm_min = 59;
        tm_end.tm_sec = 59;
        std::time_t end_time = std::mktime(&tm_end);

        std::string start_str = time_to_string(std::chrono::system_clock::from_time_t(start_time));
        std::string end_str = time_to_string(std::chrono::system_clock::from_time_t(end_time));

        json result = json::array();
        sqlite3_stmt* stmt;
        const char* sql = "SELECT timestamp, data FROM sensor_data "
                          "WHERE timestamp BETWEEN ? AND ? "
                          "ORDER BY timestamp;";

        if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, start_str.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, end_str.c_str(), -1, SQLITE_STATIC);

            while(sqlite3_step(stmt) == SQLITE_ROW) {
                const char* timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                const char* data_str = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                
                try {
                    json data = json::parse(data_str);
                    data["timestamp"] = timestamp;
                    result.push_back(data);
                }
                catch(const json::parse_error& e) {
                    std::cerr << "JSON parse error: " << e.what() << std::endl;
                }
            }
            sqlite3_finalize(stmt);
        }
        return result;
    }

private:
    std::string time_to_string(const std::chrono::system_clock::time_point& tp) {
        std::time_t time = std::chrono::system_clock::to_time_t(tp);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
        return buffer;
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

    void log(const std::string& ip, int day_number, const json& response) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char time_str[20];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));
        
        log_file << time_str 
                << " | IP: " << ip
                << " | Day number: " << day_number
                << " | Records sent: " << response.size()
                << std::endl;
    }
};

void handle_client(tcp::socket socket, Database& db, Logger& logger) {
    try {
        std::string client_ip = socket.remote_endpoint().address().to_string();
        
        asio::streambuf buf;
        asio::read_until(socket, buf, '\n');
        
        std::istream is(&buf);
        std::string request_str;
        std::getline(is, request_str);
        
        auto request = json::parse(request_str);
        if(!request.contains("data") || !request["data"].is_number()) {
            throw std::runtime_error("Invalid request format");
        }
        
        int day_number = request["data"];
        if(day_number < 1 || day_number > 31) {
            throw std::runtime_error("Day number out of range");
        }
        
        auto response = db.get_day_data(day_number);
        
        std::string response_str = response.dump() + "\n";
        asio::write(socket, asio::buffer(response_str));
        
        logger.log(client_ip, day_number, response);
        std::cout << "Sent " << response.size() 
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
