#include <iostream>
#include <sqlite3.h>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Конфигурация
const string MQTT_BROKER = "tcp://localhost:1883";
const string MQTT_TOPIC = "/data";
const string DB_FILE = "data.db";

class MQTTListener : public virtual mqtt::callback {
    sqlite3* db;
    
    void create_table() {
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS sensor_data ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
            "data TEXT);";
            
        if (sqlite3_exec(db, sql, nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw runtime_error(sqlite3_errmsg(db));
        }
    }

public:
    MQTTListener() {
        if (sqlite3_open(DB_FILE.c_str(), &db) != SQLITE_OK) {
            throw runtime_error(sqlite3_errmsg(db));
        }
        create_table();
    }

    ~MQTTListener() {
        sqlite3_close(db);
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        try {
            auto j = json::parse(msg->get_payload());
            
            sqlite3_stmt* stmt;
            const char* sql = "INSERT INTO sensor_data (data) VALUES (?);";
            
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                string data = msg->get_payload();
                sqlite3_bind_text(stmt, 1, data.c_str(), -1, SQLITE_STATIC);
                
                if (sqlite3_step(stmt) != SQLITE_DONE) {
                    cerr << "Insert error: " << sqlite3_errmsg(db) << endl;
                }
                sqlite3_finalize(stmt);
            }
        }
        catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
};

int main() {
    try {
        mqtt::async_client client(MQTT_BROKER, "mqtt2sql");
        MQTTListener listener;
        
        client.set_callback(listener);
        client.connect()->wait();
        client.subscribe(MQTT_TOPIC, 1);
        
        cout << "Service started. Press Enter to exit..." << endl;
        cin.get();
        
        client.unsubscribe(MQTT_TOPIC)->wait();
        client.disconnect()->wait();
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
