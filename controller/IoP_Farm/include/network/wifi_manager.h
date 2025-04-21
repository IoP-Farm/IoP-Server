#pragma once

#include <WiFiManager.h>
#include "utils/logger_factory.h"
#include "config/constants.h"

namespace farm::net
{
    // Используем пространство имен для констант WiFi
    using namespace farm::config::wifi;
    
    // Упрощенная обертка над WiFiManager от tzapu
    class MyWiFiManager
    {
    private:
        // Экземпляр WiFiManager от tzapu
        WiFiManager wifiManager;
        
        // Логгер
        std::shared_ptr<farm::log::ILogger> logger;
        
        // Время последней проверки соединения
        unsigned long lastCheckTime = 0;
        
        // Указывает, активен ли портал конфигурации
        bool portalActive = false;
        
        // Счетчик попыток переподключения
        uint8_t reconnectAttempts = 0;
        
        // Время последней попытки переподключения
        unsigned long lastReconnectTime = 0;
        
        // Настройки точки доступа
        String apName     = DEFAULT_AP_NAME;
        String apPassword = DEFAULT_AP_PASSWORD;
        String hostName   = DEFAULT_HOSTNAME;

    public:
        // Конструктор
        explicit MyWiFiManager(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Установка имени и пароля точки доступа
        void setAccessPointCredentials(const String& name, const String& password = DEFAULT_AP_PASSWORD);
        
        // Установка имени хоста
        void setHostName(const String& name);
        
        // Проверка наличия сохраненных настроек WiFi
        bool checkWifiSaved();
        
        // Инициализация WiFi и попытка подключения
        bool initialize();
        
        // Сброс всех настроек WiFi и перезагрузка
        bool resetSettings();
        
        // Поддержание WiFi соединения - вызывать в цикле loop()
        void maintainConnection();
        
        // Получение прямого доступа к WiFiManager для продвинутой настройки
        WiFiManager* getWiFiManager();
        
        // Проверка наличия соединения
        bool isConnected() const;
        
        // Проверка активности портала конфигурации
        bool isConfigPortalActive();
        
        // Включение/выключение встроенной отладки WiFiManager
        void setDebugOutput(bool enable);
        void setDebugOutput(bool enable, const String& prefix);
    };
}
