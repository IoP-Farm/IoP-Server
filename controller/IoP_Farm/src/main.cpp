#include <Arduino.h>
#include <memory>
#include <GyverNTP.h>

#include "network/wifi_manager.h"
#include "network/mqtt_manager.h"

#include "config/config_manager.h"
#include "config/constants.h"

#include "sensors/sensors_manager.h"

#include "utils/ota_manager.h"
#include "utils/web_server_manager.h"
#include "utils/logger_factory.h"
#include "utils/logger.h"
#include "utils/scheduler.h"

using namespace farm::config;
using namespace farm::config::sensors;
using namespace farm::log;
using namespace farm::net;
using namespace farm::sensors;
using namespace farm::utils;

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
std::shared_ptr<Scheduler>        schedulerManager = Scheduler::getInstance(logger);  


void setup() 
{
    Serial.begin(115200);
    delay(1000);

    pinMode(pins::LED_PIN, OUTPUT);
    digitalWrite(pins::LED_PIN, HIGH);

    configManager->initialize();     

    logger->log(Level::Debug, "Текущая файловая система SPIFFS:");
    #ifdef IOP_DEBUG
    configManager->printSpiffsInfo();
    #endif

    wifiManager     ->initialize();  
    mqttManager     ->initialize();  
    sensorsManager  ->initialize();  
    otaManager      ->initialize();  
    
    // Включаем аутентификацию для веб-сервера
    webServerManager->enableAuth(true);
    webServerManager->initialize();  
    
    //
    schedulerManager->initialize(time::DEFAULT_GMT_OFFSET);  
    
    // Пример планирования действий (добавьте свои события по необходимости)
    // Тестовое событие через 60 секунд после запуска
    schedulerManager->scheduleOnceAfter(scheduler::TEST_EVENT_DELAY_SEC, []() {
        logger->log(Level::Warning, "Тестовое одноразовое событие выполнено!");
        logger->log(Level::Warning, "Текущее время: %s", NTP.toString().c_str());
    });

#ifdef USE_FREERTOS
    // Запускаем планировщик с приоритетом и стеком
    schedulerManager->startSchedulerTask(scheduler::SCHEDULER_TASK_PRIORITY, 
                                         scheduler::SCHEDULER_STACK_SIZE);
#endif
}

void loop() 
{
    wifiManager->maintainConnection();
    mqttManager->maintainConnection();

    // Обслуживание датчиков    
    sensorsManager->loop();

    // Обработка NTP и планировщика
    if (NTP.tick()) 
    {
        // Выполняем действия, которые нужно выполнять каждую секунду
        // Например, вывод текущего времени
        static uint32_t lastNTPTick = 0;
        Serial.print("#");
        Serial.print(lastNTPTick++);
        Serial.print(" NTP tick: ");
        Serial.println(NTP.toString());
    }
    
#ifndef USE_FREERTOS
    // Проверка и выполнение запланированных событий только если не используем FreeRTOS
    schedulerManager->checkSchedule();
#endif

    // Обработка OTA и веб-сервера
    otaManager->handle(); 
    webServerManager->handleClient();       

    // Оставляем небольшую задержку для других задач
    delay(loop::DEFAULT_DELAY_MS);
}