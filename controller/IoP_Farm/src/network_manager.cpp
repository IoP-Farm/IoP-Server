#include "network_manager.h"

namespace farm::net
{
    // Логгер для сообщений Wi-Fi
    void WiFiLogger::logError(WiFiError error)
    {
        farm::log::SerialLogger logger;
        switch (error)
        {
            case WiFiError::Timeout:
                logger.error("[WiFi] Ошибка подключения: Превышено время ожидания");
                break;
            case WiFiError::InvalidCredentials:
                logger.error("[WiFi] Ошибка подключения: Неверные учетные данные (SSID или пароль)");
                break;
            case WiFiError::HardwareNotFound:
                logger.error("[WiFi] Ошибка подключения: Wi-Fi адаптер не найден");
                break;
            case WiFiError::AlreadyConnected:
                logger.info("[WiFi] Уже подключено к Wi-Fi");
                break;
            case WiFiError::AlreadyConnecting:
                logger.info("[WiFi] Уже идет попытка подключения к Wi-Fi");
                break;
            case WiFiError::Unknown:
            default:
                logger.error("[WiFi] Неизвестная ошибка при подключении к Wi-Fi");
                break;
        }
    }

    WiFiResult WiFiManager::connect()
    {
        // Если уже подключены, возвращаем успех без ошибок
        if (isConnected())  
        {
            WiFiLogger::logError(WiFiError::AlreadyConnected);
            return {true, WiFiError::AlreadyConnected};
        }

        // Сбрасываем все предыдущие подключения
        WiFi.disconnect(); 

        // Начинаем процесс подключения
        WiFi.mode(WIFI_STA);
        WiFi.begin(confidential::WIFI_SSID, confidential::WIFI_PASSWORD);

        farm::log::SerialLogger logger;
        logger.info("[WiFi] Подключение к Wi-Fi...");

        const unsigned long timeout = 15000; // Таймаут подключения
        unsigned long start = millis();

        // Ждем подключения в течение таймаута
        while (!isConnected() && millis() - start < timeout) 
        {
            delay(500);
            logger.debug("[WiFi] Ожидание подключения...");
        }

        // Проверка успешности подключения
        if (isConnected())  
        {
            logger.info(("[WiFi] Подключено успешно: " + WiFi.localIP().toString()).c_str());
            return {true, WiFiError::None};
        }
        else
        {
            // Выбираем тип ошибки в зависимости от статуса подключения
            WiFiError error = WiFiError::Timeout;
            switch (WiFi.status())
            {
                case WL_CONNECT_FAILED:
                    error = WiFiError::InvalidCredentials;
                    break;
                case WL_NO_SSID_AVAIL:
                    error = WiFiError::HardwareNotFound;
                    break;
                default:
                    error = WiFiError::Timeout;
                    break;
            }

            // Логируем ошибку
            WiFiLogger::logError(error);

            return {false, error}; 
        }
    }

    bool WiFiManager::isConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }
}