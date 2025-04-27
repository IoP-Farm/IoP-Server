#pragma once

#include <Arduino.h>

namespace farm::config
{
    // Пути к файлам конфигурации
    namespace paths
    {
        constexpr const char* DATA_CONFIG    = "/data.json";     // Файл конфигурации с данными от датчиков
        constexpr const char* SYSTEM_CONFIG  = "/config.json"; // Файл основной конфигурации системы
        constexpr const char* COMMAND_CONFIG = "/cmd.json";   // Файл с командами управления
        constexpr const char* MQTT_CONFIG    = "/mqtt.json"; // Файл конфигурации MQTT
        // Пути к дефолтным файлам конфигурации
        constexpr const char* DEFAULT_DATA_CONFIG    = "/default_data.json";     // Дефолтный файл с данными от датчиков
        constexpr const char* DEFAULT_SYSTEM_CONFIG  = "/default_config.json"; // Дефолтный файл системной конфигурации
        constexpr const char* DEFAULT_COMMAND_CONFIG = "/default_cmd.json";   // Дефолтный файл с командами управления
        constexpr const char* DEFAULT_MQTT_CONFIG    = "/default_mqtt.json"; // Дефолтный файл конфигурации MQTT
    }

    // Константы для MQTT
    namespace mqtt
    {
        constexpr int16_t DEFAULT_PORT = -1;                  // Порт по умолчанию (-1 означает, что порт не назначен)
        constexpr const char* DEFAULT_HOST = "";              // Хост по умолчанию (пустая строка означает, что хост не назначен)
        
        constexpr const char* DEFAULT_DEVICE_ID = "farm001";  // ID устройства по умолчанию
        
        // Суффиксы топиков
        constexpr const char* DATA_SUFFIX    = "/data";          // Суффикс для топика данных
        constexpr const char* CONFIG_SUFFIX  = "/config";      // Суффикс для топика конфигурации
        constexpr const char* COMMAND_SUFFIX = "/command";    // Суффикс для топика команд
        
        // Константы для работы с MQTT подключением
        constexpr unsigned long CHECK_INTERVAL = 5000;           // 5 секунд между проверками соединения
        constexpr uint8_t MAX_RECONNECT_ATTEMPTS = 1;           // Максимальное количество попыток переподключения
        constexpr unsigned long RECONNECT_RETRY_INTERVAL = 10000; // 10 секунд между попытками переподключения
        
        // Уровни QoS (Quality of Service)
        constexpr uint8_t QOS_0 = 0;  // At most once - сообщение доставляется максимум один раз (или не доставляется вообще)
        constexpr uint8_t QOS_1 = 1;  // At least once - сообщение доставляется минимум один раз (возможны дубликаты)
        constexpr uint8_t QOS_2 = 2;  // Exactly once - сообщение доставляется ровно один раз (без дубликатов)
    }

    // Константы для WiFi
    namespace wifi
    {
        constexpr const char* DEFAULT_AP_NAME     = "IoP-Farm_001";   // Имя точки доступа по умолчанию
        constexpr const char* DEFAULT_AP_PASSWORD = "12345678"; // Пароль точки доступа по умолчанию
        constexpr const char* DEFAULT_HOSTNAME    = "IoP-Farm_001";  // Имя хоста по умолчанию
        
        // Константы подключения
        constexpr unsigned int CONNECT_TIMEOUT = 10;          // Таймаут подключения (секунды)
        constexpr unsigned int PORTAL_TIMEOUT = 0;            // Бесконечное ожидание в портале (0 = бесконечно)
        constexpr unsigned long CHECK_INTERVAL = 5000;        // 5 секунд между проверками соединения
        constexpr uint8_t MAX_RECONNECT_ATTEMPTS = 3;         // Максимальное количество попыток переподключения
        constexpr unsigned long RECONNECT_RETRY_INTERVAL = 5000; // 5 секунд между попытками переподключения

        // Константы для MQTT параметров в портале конфигурации
        constexpr const char* MQTT_SERVER_PARAM_ID = "mqtt_server";    // ID параметра сервера MQTT
        constexpr const char* MQTT_SERVER_PARAM_LABEL = "MQTT Server"; // Метка параметра сервера MQTT
        constexpr size_t MQTT_SERVER_PARAM_LENGTH = 40;                // Максимальная длина адреса сервера
        constexpr const char* MQTT_PORT_PARAM_ID = "mqtt_port";        // ID параметра порта MQTT
        constexpr const char* MQTT_PORT_PARAM_LABEL = "MQTT Port";     // Метка параметра порта MQTT
        constexpr size_t MQTT_PORT_PARAM_LENGTH = 6;                   // Максимальная длина порта
        constexpr const char* MQTT_DEFAULT_PORT = "1883";              // Порт MQTT по умолчанию
    }

    // Константы для JSON документов
    namespace json
    {
        
    }

    // Константы для датчиков
    namespace sensors
    {   
        // Пины для датчиков
        constexpr uint8_t DHT22_PIN = 15;                     // Пин DHT22 (темп. и влажность)
        constexpr uint8_t DS18B20_PIN = 4;                    // Пин DS18B20 (темп. воды)
        constexpr uint8_t ULTRASONIC_TRIG_PIN = 25;           // Пин HC-SR04 Trig
        constexpr uint8_t ULTRASONIC_ECHO_PIN = 26;           // Пин HC-SR04 Echo
        constexpr uint8_t SOIL_MOISTURE_PIN = 32;             // Пин FC-28 (влажность почвы)
        constexpr uint8_t LIGHT_SENSOR_PIN = 33;              // Пин KY-018 (освещённость)
    }

    // Константы для исполнительных устройств
    namespace actuators
    {
        // Пины для исполнительных устройств
        constexpr uint8_t PUMP_PIN = 12;                      // Пин помпы
        constexpr uint8_t GROW_LIGHT_PIN = 13;                // Пин фитолампы
        constexpr uint8_t HEAT_LAMP_PIN = 14;                 // Пин нагревательной лампы
        
        // Константы нагревателя
        constexpr float HEATER_HYSTERESIS = 1.0f;             // Гистерезис нагревателя (°C)
    }
    
    // Команды управления, используемые в топике MQTT command
    enum class CommandCode : uint8_t
    {
        ESP_RESTART    = 0,  // Аварийный перезапуск ESP32
        PUMP_ON        = 1,  // Включить насос
        PUMP_OFF       = 2,  // Выключить насос
        GROWLIGHT_ON   = 3,  // Включить фитолампу
        GROWLIGHT_OFF  = 4,  // Выключить фитолампу
        HEATLAMP_ON    = 5,  // Включить лампу нагрева
        HEATLAMP_OFF   = 6   // Выключить лампу нагрева
    };
} 