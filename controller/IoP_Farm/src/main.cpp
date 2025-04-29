#include <Arduino.h>
#include "network/wifi_manager.h"
#include "network/mqtt_manager.h"
#include "config/config_manager.h"
#include "utils/logger_factory.h"
#include "sensors/sensors_manager.h"

#ifdef IOP_DEBUG
#include "unit/test_config_manager.h"
#endif

using namespace farm::config;
using namespace farm::log;
using namespace farm::net;
using namespace farm::sensors;
using namespace farm::config::sensors::timing;

// Создаем логгер
#ifdef IOP_DEBUG
auto logger = LoggerFactory::createSerialLogger(Level::Debug);
#else
auto logger = LoggerFactory::createSerialLogger(Level::Info);
#endif

// Получаем экземпляры синглтонов
std::shared_ptr<ConfigManager> configManager = ConfigManager::getInstance(logger);
std::shared_ptr<MyWiFiManager> wifiManager   = MyWiFiManager::getInstance(logger);
std::shared_ptr<MQTTManager>   mqttManager   = MQTTManager::getInstance(logger);
std::shared_ptr<SensorsManager> sensorsManager = SensorsManager::getInstance(logger);

void setup() 
{
    // Инициализация Serial
    Serial.begin(115200);
    delay(1000);

    configManager->initialize(); // Инициализация ConfigManager

    logger->log(Level::Debug, "Текущая файловая система SPIFFS:");
    configManager->printSpiffsInfo();

    wifiManager  ->initialize(); // Инициализация WiFiManager
    mqttManager  ->initialize(); // Инициализация MQTTManager
    sensorsManager->initialize(); // Инициализация SensorsManager
    
    // Устанавливаем интервал считывания датчиков
    sensorsManager->setReadInterval(DEFAULT_READ_INTERVAL);
}

void loop() 
{
    wifiManager->maintainConnection();
    mqttManager->maintainConnection();
    sensorsManager->loop(); // Обслуживание датчиков

    // Оставляем небольшую задержку для других задач
    delay(100);
}