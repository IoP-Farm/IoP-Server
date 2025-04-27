#include <Arduino.h>
#include "network/wifi_manager.h"
#include "network/mqtt_manager.h"
#include "config/config_manager.h"
#include "utils/logger_factory.h"

#ifdef IOP_DEBUG
#include "unit/test_config_manager.h"
#endif

using namespace farm::config;
using namespace farm::log;
using namespace farm::net;

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

void setup() 
{
    // Инициализация Serial
    Serial.begin(115200);
    delay(1000);

    configManager->initialize(); // Инициализация ConfigManager
    wifiManager  ->initialize(); // Инициализация WiFiManager
    mqttManager  ->initialize(); // Инициализация MQTTManager
}

void loop() 
{
    wifiManager->maintainConnection();
    mqttManager->maintainConnection();

    // Каждые 20 секунд отправляем сообщение в MQTT
    if (mqttManager->isClientConnected()) 
    {
        static unsigned long lastMqttPublish = 0;
        if (millis() - lastMqttPublish >= 20000) 
        {
            lastMqttPublish = millis();
            mqttManager->publishData();
        }
    }

    // Выполнение других задач проекта
    delay(100);
}