#include <Arduino.h>
#include "network/wifi_manager.h"
#include "utils/logger_factory.h"
#include <vector>

// Создаем логгер и WiFiManager
#ifdef IOP_DEBUG
auto logger = farm::log::LoggerFactory::createSerialLogger(farm::log::Level::Debug);
#else
auto logger = farm::log::LoggerFactory::createSerialLogger(farm::log::Level::Info);
#endif

farm::net::MyWiFiManager wifiManager(logger);

void setup() 
{
    // Инициализация Serial
    Serial.begin(115200);
    delay(1000);
    
    // Инициализация WiFi с автоподключением
    wifiManager.initialize();
}

void loop() 
{
    // Поддержание WiFi соединения
    wifiManager.maintainConnection();

    // Выполнение других задач проекта
    delay(100);
} 