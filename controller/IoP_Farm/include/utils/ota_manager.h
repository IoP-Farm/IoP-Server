#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "utils/logger_factory.h"
#include "config/constants.h"  // Добавляем включение констант
#include <memory>

namespace farm::net
{
    using namespace farm::config;  // Используем пространство имен для констант

    // Класс для управления OTA обновлениями
    class OTAManager 
    {
    private:
        // Приватный конструктор (паттерн Синглтон)
        explicit OTAManager(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Статический экземпляр
        static std::shared_ptr<OTAManager> instance;
        
        // Логгер
        std::shared_ptr<farm::log::ILogger> logger;
        
    public:
        // Получение экземпляра синглтона
        static std::shared_ptr<OTAManager> getInstance(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Запрещаем копирование и присваивание
        OTAManager(const OTAManager&) = delete;
        OTAManager& operator=(const OTAManager&) = delete;
        
        // Деструктор
        ~OTAManager() = default;
        
        // Инициализация OTA
        bool initialize();
        
        // Установка имени хоста (переопределяет DEFAULT_HOSTNAME)
        void setHostname(const String& name);
        
        // Установка пароля
        void setPassword(const String& password);
        
        // Обработка OTA-запросов, вызывать в цикле loop()
        void handle();
    };
}