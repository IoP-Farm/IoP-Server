#include <Arduino.h>
#include <memory>

#include "sensors/sensors_manager.h"

#include "network/wifi_manager.h"
#include "network/mqtt_manager.h"

#include "config/config_manager.h"
#include "config/constants.h"

#include "utils/logger_factory.h"
#include "utils/logger.h"
#include "utils/ota_manager.h"
#include "utils/web_server_manager.h"

using namespace farm::config;
using namespace farm::config::sensors;
using namespace farm::log;
using namespace farm::net;
using namespace farm::sensors;

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
std::shared_ptr<ConfigManager>    configManager    = ConfigManager::getInstance(logger);
std::shared_ptr<MyWiFiManager>    wifiManager      = MyWiFiManager::getInstance(logger);
std::shared_ptr<MQTTManager>      mqttManager      = MQTTManager::getInstance(logger);
std::shared_ptr<SensorsManager>   sensorsManager   = SensorsManager::getInstance(logger);
std::shared_ptr<OTAManager>       otaManager       = OTAManager::getInstance(logger);
std::shared_ptr<WebServerManager> webServerManager = WebServerManager::getInstance(logger);


void setup() 
{
    // Инициализация Serial
    Serial.begin(115200);
    delay(1000);

    pinMode(pins::LED_PIN, OUTPUT);
    digitalWrite(pins::LED_PIN, HIGH);

    configManager->initialize(); // Инициализация ConfigManager

    logger->log(Level::Debug, "Текущая файловая система SPIFFS:");
    #ifdef IOP_DEBUG
    configManager->printSpiffsInfo();
    #endif

    wifiManager     ->initialize(); // Инициализация WiFiManager
    mqttManager     ->initialize(); // Инициализация MQTTManager
    sensorsManager  ->initialize(); // Инициализация SensorsManager
    otaManager      ->initialize(); // Инициализация OTAManager
    
    // Инициализация WebServerManager (с аутентификацией)
    webServerManager->enableAuth(true);
    webServerManager->initialize(); // Инициализация WebServerManager
    
    // Устанавливаем интервал считывания датчиков
    sensorsManager->setReadInterval(timing::DEFAULT_READ_INTERVAL);
}

void loop() 
{
    wifiManager->maintainConnection();
    mqttManager->maintainConnection();

    // Обслуживание датчиков    
    sensorsManager->loop();

    // Обработка OTA и веб-сервера
    otaManager->handle(); 
    webServerManager->handleClient();       

    // Оставляем небольшую задержку для других задач
    delay(100);
}