#pragma once

#include <Arduino.h>

namespace farm::config
{
    // Типы конфигурационных файлов
    enum class ConfigType
    {
        Data,           // Данные от сенсоров ({device_id}/data)
        System,         // Конфигурация системы ({device_id}/config)
        Command,        // Команды управления ({device_id}/command)
        Mqtt            // Конфигурация MQTT (не отправляется в топики! Нужно для сохранения в памяти)
    };
    
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
        constexpr uint8_t DEVICE_ID_MAX_LENGTH = 32;      // Максимальная длина имени устройства
        
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
        namespace pins
        {
            constexpr int8_t DHT22_PIN           =  4;  // Пин DHT22 (темп. и влажность) ex: 4
            constexpr int8_t DS18B20_PIN         = 13;  // Пин DS18B20 (темп. воды) ex: 13
            constexpr int8_t HC_SR04_TRIG_PIN    =  5;  // Пин HC-SR04 Trig ex: 5
            constexpr int8_t HC_SR04_ECHO_PIN    = 18;  // Пин HC-SR04 Echo ex: 18
            constexpr int8_t FC28_PIN            = 36;  // Пин FC-28 (влажность почвы) ex: 36
            constexpr int8_t KY018_PIN           = 39;  // Пин KY-018 (освещённость) ex: 39
            constexpr int8_t YFS401_PIN          = 22;  // Пин YF-S401 (расходомер) ex: 22
        }

        // Имена датчиков
        namespace names
        {
            constexpr const char* DS18B20 = "DS18B20";                // Датчик температуры воды
            constexpr const char* DHT22_TEMPERATURE = "DHT22_Temp";   // Датчик температуры воздуха
            constexpr const char* DHT22_HUMIDITY = "DHT22_Humidity";  // Датчик влажности воздуха
            constexpr const char* HCSR04 = "HC-SR04";                 // Ультразвуковой датчик
            constexpr const char* FC28 = "FC-28";                     // Датчик влажности почвы
            constexpr const char* KY018 = "KY-018";                   // Датчик освещенности
            constexpr const char* YFS401 = "YF-S401";                 // Расходомер воды
        }

        // JSON ключи для типов измерений
        namespace json_keys
        {
            constexpr const char* TEMPERATURE_DHT22 = "temperature_DHT22";     // Температура воздуха
            constexpr const char* TEMPERATURE_DS18B20 = "temperature_DS18B20"; // Температура воды
            constexpr const char* HUMIDITY = "humidity";                       // Влажность воздуха
            constexpr const char* WATER_LEVEL = "water_level";                 // Уровень воды
            constexpr const char* SOIL_MOISTURE = "soil_moisture";             // Влажность почвы
            constexpr const char* LIGHT_INTENSITY = "light_intensity";         // Интенсивность освещения
            constexpr const char* WATER_FLOW = "water_flow";                   // Расход воды
        }

        // Единицы измерения
        namespace units
        {
            constexpr const char* CELSIUS = "°C";          // Градусы Цельсия
            constexpr const char* PERCENT = "%";           // Проценты
            constexpr const char* CENTIMETER = "см";       // Сантиметры
            constexpr const char* LITER_PER_MINUTE = "л/мин"; // Литры в минуту
            constexpr const char* MILLILITERS = "мл";       // Миллилитры
            constexpr const char* LITERS = "л";             // Литры
        }

        // Калибровочные и другие константы для датчиков
        namespace calibration
        {
            // Для датчика влажности почвы FC-28
            constexpr int FC28_DRY_VALUE = 4095;   // Значение АЦП для сухой почвы
            constexpr int FC28_WET_VALUE = 0;      // Значение АЦП для влажной почвы
            
            // Для датчика освещенности KY-018
            constexpr int KY018_DARK_VALUE = 4095; // Значение АЦП в темноте
            constexpr int KY018_LIGHT_VALUE = 0;   // Значение АЦП при ярком свете
            
            // Для расходомера воды YF-S401
            constexpr float YFS401_CALIBRATION_FACTOR = 450.0f; // Импульсов на литр
            
            // Для ультразвукового датчика HC-SR04
            constexpr float HCSR04_DEFAULT_CONTAINER_DEPTH = 20.0f; // Стандартная глубина в см

            // Какая доля глубины бака (в %) считается за 100% наполнения
            // формула: порог уровня воды(%) = (порог уровня воды(см) / глубина бака(см)) * 100%
            constexpr float HCSR04_FULL_TANK_PERCENT = (16.0f / HCSR04_DEFAULT_CONTAINER_DEPTH) * 100.0f; 
            // Скорость звука в воздухе при 20°C = 343 м/с = 0.0343 см/мкс
            constexpr float SOUND_SPEED = 343.0f * 0.01f;
            
            // Ошибочное значение для несинициализированных датчиков
            constexpr float SENSOR_ERROR_VALUE = -100.0f;

            // Неинициализированные пины
            constexpr int8_t UNINITIALIZED_PIN = -1;

            // Нет данных
            constexpr float NO_DATA = -50.0f;
        }
        
        // Временные константы для сенсоров
        namespace timing
        {
            constexpr unsigned long DEFAULT_READ_INTERVAL = 10000; // 10 секунд между считываниями с датчиков
        }
    }

    // Константы для исполнительных устройств
    namespace actuators
    {
        
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