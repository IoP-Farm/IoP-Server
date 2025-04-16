#pragma once
#include <WiFi.h>
#include "confidential.h"
#include "logger.h"

namespace farm::net
{
    // Перечисление ошибок подключения к Wi-Fi
    enum class WiFiError
    {
        None,
        Timeout,
        InvalidCredentials,
        HardwareNotFound,
        AlreadyConnected,
        AlreadyConnecting,
        Unknown
    };

    // Структура результата подключения
    struct WiFiResult
    {
        bool success;      // Успешность подключения
        WiFiError error;   // Тип ошибки подключения
    };

    // Логгер для сообщений Wi-Fi
    class WiFiLogger
    {
    public:
        static void logError(WiFiError error);  // Логирование ошибок
    };

    // Менеджер для подключения и управления Wi-Fi
    class WiFiManager
    {
    public:
        WiFiResult connect();  // Подключение к Wi-Fi
        bool isConnected() const;  // Проверка успешности подключения
    private:
        bool isConnecting = false;  // Статус подключения (не используется в новой версии)
    };
}