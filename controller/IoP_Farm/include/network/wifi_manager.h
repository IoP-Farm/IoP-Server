#pragma once

#include <WiFiManager.h>
#include "../utils/logger_factory.h"

namespace farm::net
{
    // Константы для работы с WiFi
    constexpr unsigned int WIFI_CONNECT_TIMEOUT = 10;      // Таймаут подключения (секунды)
    constexpr unsigned int WIFI_PORTAL_TIMEOUT = 0;        // Бесконечное ожидание в портале (0 = бесконечно)
    constexpr unsigned long WIFI_CHECK_INTERVAL = 5000;    // 5 секунд между проверками соединения
    constexpr uint8_t MAX_RECONNECT_ATTEMPTS = 3;          // Максимальное количество попыток переподключения
    constexpr unsigned long RECONNECT_RETRY_INTERVAL = 5000; // 5 секунд между попытками переподключения

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
        String apName = "IoP-Farm";
        String apPassword = "";
        String hostName = "";

    public:
        // Конструктор
        explicit MyWiFiManager(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Установка имени и пароля точки доступа
        void setAccessPointCredentials(const String& name, const String& password = "");
        
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
