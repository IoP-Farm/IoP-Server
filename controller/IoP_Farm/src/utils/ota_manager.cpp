#include "utils/ota_manager.h"

namespace farm::net
{
    // Инициализация статической переменной
    std::shared_ptr<OTAManager> OTAManager::instance = nullptr;
    
    // Конструктор
    OTAManager::OTAManager(std::shared_ptr<farm::log::ILogger> logger)
        : logger(logger) 
    {
        if (!logger) {
            this->logger = farm::log::LoggerFactory::createSerialLogger();
        }
    }
    
    // Получение экземпляра синглтона
    std::shared_ptr<OTAManager> OTAManager::getInstance(std::shared_ptr<farm::log::ILogger> logger)
    {
        if (!instance) {
            instance = std::shared_ptr<OTAManager>(new OTAManager(logger));
        }
        return instance;
    }
    
    // Инициализация OTA
    bool OTAManager::initialize()
    {
        // Установка имени хоста из константы
        ArduinoOTA.setHostname(wifi::DEFAULT_HOSTNAME);
        
        // Обработчики ArduinoOTA
        ArduinoOTA.onStart([this]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";
            logger->log(farm::log::Level::Warning, "[ArduinoOTA] начало обновления %s", type.c_str());
        });
        
        ArduinoOTA.onEnd([this]() {
            Serial.println("[ArduinoOTA] завершение обновления");
        });
        
        ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
            Serial.printf("[ArduinoOTA] прогресс %u%%\n", (progress / (total / 100)));
        });
        
        ArduinoOTA.onError([this](ota_error_t error) {
            if (error == OTA_AUTH_ERROR) 
                Serial.printf("[ArduinoOTA] ошибка авторизации\n");
            else if (error == OTA_BEGIN_ERROR) 
                Serial.printf("[ArduinoOTA] ошибка начала обновления\n");
            else if (error == OTA_CONNECT_ERROR) 
                Serial.printf("[ArduinoOTA] ошибка соединения\n");
            else if (error == OTA_RECEIVE_ERROR) 
                Serial.printf("[ArduinoOTA] ошибка получения данных\n");
            else if (error == OTA_END_ERROR) 
                Serial.printf("[ArduinoOTA] ошибка завершения\n");
        });
        
        // Запуск OTA
        ArduinoOTA.begin();

        setPassword(wifi::DEFAULT_AP_PASSWORD);
        logger->log(farm::log::Level::Farm, "[ArduinoOTA] ArduinoOTA инициализирован");
        
        return true;
    }
    
    // Установка имени хоста
    void OTAManager::setHostname(const String& name)
    {
        ArduinoOTA.setHostname(name.c_str());
    }
    
    // Установка пароля
    void OTAManager::setPassword(const String& password)
    {
        ArduinoOTA.setPassword(password.c_str());
    }
    
    // Обработка OTA-запросов
    void OTAManager::handle()
    {
        ArduinoOTA.handle();
    }
}