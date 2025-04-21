#include <Arduino.h>
#include "network/wifi_manager.h"
#include "utils/logger_factory.h"
#include "config/config_manager.h"
#include <vector>

#ifdef IOP_DEBUG
#include "unit/test_config_manager.h"
#endif

using namespace farm::config;
using namespace farm::log;
using namespace farm::net;

// Создаем логгер и WiFiManager
#ifdef IOP_DEBUG
auto logger = LoggerFactory::createSerialLogger(Level::Debug);
#else
auto logger = LoggerFactory::createSerialLogger(Level::Info);
#endif

MyWiFiManager wifiManager(logger);
std::shared_ptr<ConfigManager> configManager = ConfigManager::getInstance(logger);

void setup() 
{
    // Инициализация Serial
    Serial.begin(115200);
    delay(1000);

    // Инициализация хранилища конфигураций
    configManager->initialize();
    
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