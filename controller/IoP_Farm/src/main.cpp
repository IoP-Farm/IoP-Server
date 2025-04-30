#include <Arduino.h>
#include "network/wifi_manager.h"
#include "network/mqtt_manager.h"
#include "config/config_manager.h"
#include "utils/logger_factory.h"
#include "sensors/sensors_manager.h"
#include "utils/logger.h"
#include "memory"

using namespace farm::config;
using namespace farm::log;
using namespace farm::net;
using namespace farm::sensors;
using namespace farm::config::sensors::timing;

/*
Горит светодиодом при:
- Нет подключения к WiFi
- Нет подключения к MQTT
*/

/*
Мигает светодиодом при:
- Публикации данных в MQTT
- Получении сообщений от MQTT
*/

// Создаем логгер
#if defined(IOP_DEBUG) && defined(COLOR_SERIAL_LOG)
auto logger = LoggerFactory::createColorSerialMQTTLogger(Level::Debug);
#elif defined(IOP_DEBUG)
auto logger = LoggerFactory::createSerialMQTTLogger(Level::Debug);
#elif defined(COLOR_SERIAL_LOG)
auto logger = LoggerFactory::createColorSerialMQTTLogger(Level::Info);
#else
auto logger = LoggerFactory::createSerialMQTTLogger(Level::Info);
#endif


// Получаем экземпляры синглтонов
std::shared_ptr<ConfigManager>  configManager = ConfigManager::getInstance(logger);
std::shared_ptr<MyWiFiManager>  wifiManager   = MyWiFiManager::getInstance(logger);
std::shared_ptr<MQTTManager>    mqttManager   = MQTTManager::getInstance(logger);
std::shared_ptr<SensorsManager> sensorsManager = SensorsManager::getInstance(logger);


void setup() 
{
    // Инициализация Serial
    Serial.begin(115200);
    delay(1000);

    pinMode(pins::LED_PIN, OUTPUT);

    configManager->initialize(); // Инициализация ConfigManager

    logger->log(Level::Debug, "Текущая файловая система SPIFFS:");
    #ifdef IOP_DEBUG
    configManager->printSpiffsInfo();
    #endif

    wifiManager   ->initialize(); // Инициализация WiFiManager
    mqttManager   ->initialize(); // Инициализация MQTTManager
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