#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h> // Подключаем библиотеку для работы с MQTT
#include "logger.h"
#include "network_manager.h"
#include "confidential.h" // Для конфиденциальных данных (Wi-Fi и MQTT)

using farm::log::Level;
using farm::log::SerialLogger;
using farm::net::WiFiManager;
using farm::net::WiFiResult;

SerialLogger logger;        // Глобальный экземпляр логгера
WiFiManager wifi;           // Экземпляр WiFi-менеджера

WiFiClient espClient;       // Клиент для подключения к Wi-Fi
PubSubClient mqttClient(espClient); // Клиент для подключения к MQTT

// Функция для подключения к MQTT серверу
bool connectToMQTT() {
    mqttClient.setServer(confidential::MQTT_SERVER, 1883); // Указываем MQTT сервер и порт

    // Подключаемся к MQTT серверу
    if (mqttClient.connect("ESP32_Client")) { // "ESP32_Client" - это клиентский ID
        logger.info("[MQTT] Подключение к серверу успешно.");
        return true;
    } else {
        logger.error(("[MQTT] Ошибка подключения к серверу. Код ошибки: " + String(mqttClient.state())).c_str());
        return false;
    }
}

void setup()
{
    Serial.begin(115200);       // Инициализация Serial
    logger.setLevel(Level::Debug); // Устанавливаем подробный уровень логирования

    // Подключение к Wi-Fi
    WiFiResult result = wifi.connect();
    if (!result.success)
    {
        logger.error(("Остановка из-за ошибки подключения: " + String(static_cast<int>(result.error))).c_str());
        ESP.restart();
    }

    // Подключение к MQTT серверу
    if (!connectToMQTT()) {
        logger.error("[MQTT] Ошибка подключения к MQTT. Перезагружаем...");
        ESP.restart();
    }
}

void loop()
{
    if (!mqttClient.connected()) {
        logger.debug("[MQTT] Потеряно подключение, повторное подключение...");
        if (connectToMQTT()) {
            logger.debug("[MQTT] Подключение восстановлено.");
        }
    }

    mqttClient.loop();  // Обрабатываем сообщения MQTT
    delay(5000);  // TODO сделать в const Периодический вывод, чтобы проверить состояние подключен
}
