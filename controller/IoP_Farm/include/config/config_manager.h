#pragma once

#include <ArduinoJson.h>  // Используется ArduinoJson 7.x
#include <FS.h>
#include <SPIFFS.h>
#include <memory>
#include <optional>
#include "utils/logger_factory.h"
#include "constants.h"

// TODO: сейчас команда для начальной загрузки файлов конфигурации в память: pio run --target uploadfs

/*
    Для загрузки файлов в SPIFFS: pio run --target uploadfs
    Для скачивания содержимого SPIFFS можно использовать: pio run --target downloadfs
*/

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

    // Менеджер конфигурации - синглтон с использованием std::shared_ptr
    class ConfigManager
    {
    private:
        // Приватный конструктор (паттерн Синглтон)
        explicit ConfigManager(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Статический экземпляр как shared_ptr
        static std::shared_ptr<ConfigManager> instance;
        
        // Логгер
        std::shared_ptr<farm::log::ILogger> logger;
        
        // JSON документы для разных типов конфигураций
        JsonDocument dataConfig;     // Данные от датчиков
        JsonDocument systemConfig;   // Системная конфигурация
        JsonDocument commandConfig;  // Команды управления

        JsonDocument mqttConfig;     // Конфигурация MQTT
        
        // Методы для работы с конфигурацией
        bool loadJsonFromFile(const char* path, JsonDocument& doc);       // Из памяти в JSON документ
        bool saveJsonToFile(const char* path, const JsonDocument& doc);   // Из JSON документа в память
        
        const char* getConfigPath(ConfigType type) const;
        const char* getDefaultConfigPath(ConfigType type) const;

        JsonDocument& getConfigDocument(ConfigType type);
        const JsonDocument& getConfigDocument(ConfigType type) const;
        
    public:
        // Получение экземпляра синглтона как shared_ptr
        static std::shared_ptr<ConfigManager> getInstance(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Запрещаем копирование и присваивание
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager& operator=(const ConfigManager&) = delete;
        
        // Деструктор
        ~ConfigManager();
        
        // Инициализация SPIFFS
        bool initialize();
        
        // Методы для работы с файлами конфигурации
        bool loadConfig(ConfigType type);
        bool saveConfig(ConfigType type);
        
        // Загрузка/сохранение всех конфигураций
        bool loadAllConfigs();
        bool saveAllConfigs();

        // Очистка конфигурации указанного типа
        bool clearConfig(ConfigType type);
        bool clearAllConfigs();

        // Удаление конфигурации указанного типа
        bool deleteConfig(ConfigType type);
        bool deleteAllConfigs();

        // Загрузка дефолтной конфигурации
        bool loadDefaultConfig(ConfigType type);
        bool loadAllDefaultConfigs();

        // Шаблонный метод для получения значения
        template<typename T>
        T getValue(ConfigType type, const char* key) const
        {
            const auto& doc = getConfigDocument(type);
            
            if (doc[key].isNull()) {
                logger->log(farm::log::Level::Error, 
                          "[Config] Ключ '%s' не найден", key);
            }
            
            return doc[key].as<T>();
        }
        
        // Шаблонный метод для установки значения
        template<typename T>
        void setValue(ConfigType type, const char* key, const T& value)
        {
            auto& doc = getConfigDocument(type);
            doc[key] = value;
            
            logger->log(farm::log::Level::Debug, 
                      "[Config] Установлено значение для ключа '%s'", key);
        }
        
        // Проверка существования ключа
        bool hasKey(ConfigType type, const char* key) const;
        
        // Вывод текущей конфигурации в Serial для отладки
        void printConfig(ConfigType type) const;
        
        // Получение JSON строки для указанного типа конфигурации
        String getConfigJson(ConfigType type) const;
        
        // Обновление конфигурации из JSON строки
        bool updateFromJson(ConfigType type, const String& jsonString);
    };
}
