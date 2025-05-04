#pragma once

#include <WiFiManager.h>             // https://github.com/tzapu/WiFiManager
#include "utils/logger_factory.h"
#include "config/constants.h"
#include <memory>

namespace farm::net
{
    // Используем пространство имен для констант WiFi
    using namespace farm::config::wifi;
    
    // Упрощенная обертка над WiFiManager от tzapu
    class MyWiFiManager
    {
    private:
        // Приватный конструктор (паттерн Синглтон)
        explicit MyWiFiManager(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Статический экземпляр как shared_ptr
        static std::shared_ptr<MyWiFiManager> instance;
        
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
        
        // Параметры для MQTT в портале конфигурации
        WiFiManagerParameter mqttServerParam;
        WiFiManagerParameter mqttPortParam;
        
        // Колбэк для сохранения параметров
        void saveParamsCallback();

    public:
        // Получение экземпляра синглтона как shared_ptr
        static std::shared_ptr<MyWiFiManager> getInstance(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Запрещаем копирование и присваивание
        MyWiFiManager(const MyWiFiManager&) = delete;
        MyWiFiManager& operator=(const MyWiFiManager&) = delete;
        
        // Деструктор
        ~MyWiFiManager();
        
        // Установка имени и пароля точки доступа
        void setAccessPointCredentials(const String& name, const String& password = DEFAULT_AP_PASSWORD);
        
        // Установка имени хоста
        void setHostName(const String& name);

        // Получение IP-адреса
        String getIPAddress() const;
        
        // Проверка наличия сохраненных настроек WiFi
        bool checkWifiSaved();
        
        // Инициализация WiFi и попытка подключения
        bool initialize();
        
        // Сброс всех настроек WiFi и перезагрузка
        bool resetSettings();
        
        // Поддержание WiFi соединения - вызывать в цикле loop()
        void maintainConnection();
        
        // Проверка наличия соединения
        bool isConnected() const;
        
        // Проверка активности портала конфигурации
        bool isConfigPortalActive();
        
        // Включение/выключение встроенной отладки WiFiManager
        void setDebugOutput(bool enable);
        void setDebugOutput(bool enable, const String& prefix);
    };
}
