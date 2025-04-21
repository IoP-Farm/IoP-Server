#include "config/config_manager.h"

// TODO: сейчас команда для начальной загрузки файлов конфигурации в память: pio run --target uploadfs

/*
    Для загрузки файлов в SPIFFS: pio run --target uploadfs
    Для скачивания содержимого SPIFFS можно использовать: pio run --target downloadfs
*/

namespace farm::config
{
    // Инициализация статического экземпляра
    std::shared_ptr<ConfigManager> ConfigManager::instance = nullptr;

    // Конструктор
    ConfigManager::ConfigManager(std::shared_ptr<farm::log::ILogger> logger)
        : dataConfig(),     // В ArduinoJson 7 не нужно указывать размер
          systemConfig(),   // В ArduinoJson 7 не нужно указывать размер
          commandConfig(),  // В ArduinoJson 7 не нужно указывать размер
          mqttConfig()      // Конфигурация MQTT
    {
        // Если логгер не передан, создаем новый с помощью фабрики
        if (!logger) 
        {
            this->logger = farm::log::LoggerFactory::createSerialLogger(farm::log::Level::Info);
        } 
        else 
        {
            this->logger = logger;
        }
        
        this->logger->log(farm::log::Level::Info, "[Config] Инициализация менеджера конфигурации");
    }

    // Получение экземпляра синглтона
    std::shared_ptr<ConfigManager> ConfigManager::getInstance(std::shared_ptr<farm::log::ILogger> logger)
    {
        if (instance == nullptr) 
        {
            // Используем явное создание вместо make_shared, т.к. конструктор приватный
            instance = std::shared_ptr<ConfigManager>(new ConfigManager(logger));
        }
        return instance;
    }

    ConfigManager::~ConfigManager()
    {
        logger->log(farm::log::Level::Debug, "[Config] Освобождение памяти");

        // Освобождаем все JSON документы
        dataConfig.clear();
        systemConfig.clear();
        commandConfig.clear();
        mqttConfig.clear();

        logger->log(farm::log::Level::Debug, "[Config] Освобождение памяти завершено");
    }

    // Инициализация SPIFFS
    bool ConfigManager::initialize()
    {
        logger->log(farm::log::Level::Info, "[Config] Инициализация файловой системы");
        
        if (!SPIFFS.begin(true)) {
            logger->log(farm::log::Level::Error, "[Config] Ошибка инициализации SPIFFS");
            return false;
        }
        
        logger->log(farm::log::Level::Info, "[Config] SPIFFS инициализирована успешно");
        
        // Проверяем наличие всех необходимых файлов
        bool allFilesExist = true;
        
        // Проверка дефолтных файлов конфигурации
        if (!SPIFFS.exists(paths::DEFAULT_DATA_CONFIG)) {
            logger->log(farm::log::Level::Error, 
                       "[Config] Отсутствует дефолтный файл конфигурации данных: %s", 
                       paths::DEFAULT_DATA_CONFIG);
            allFilesExist = false;
        }
        
        if (!SPIFFS.exists(paths::DEFAULT_SYSTEM_CONFIG)) {
            logger->log(farm::log::Level::Error, 
                       "[Config] Отсутствует дефолтный файл системной конфигурации: %s", 
                       paths::DEFAULT_SYSTEM_CONFIG);
            allFilesExist = false;
        }
        
        if (!SPIFFS.exists(paths::DEFAULT_COMMAND_CONFIG)) {
            logger->log(farm::log::Level::Error, 
                       "[Config] Отсутствует дефолтный файл конфигурации команд: %s", 
                       paths::DEFAULT_COMMAND_CONFIG);
            allFilesExist = false;
        }
        
        if (!SPIFFS.exists(paths::DEFAULT_MQTT_CONFIG)) {
            logger->log(farm::log::Level::Error, 
                       "[Config] Отсутствует дефолтный файл конфигурации MQTT: %s", 
                       paths::DEFAULT_MQTT_CONFIG);
            allFilesExist = false;
        }

        if (!allFilesExist) {
            logger->log(farm::log::Level::Error, 
                       "[Config] Отсутствуют необходимые дефолтные файлы конфигурации. "
                       "Убедитесь, что файлы загружены в SPIFFS с помощью команды: "
                       "pio run --target uploadfs");
            return false;
        }

        logger->log(farm::log::Level::Info, "[Config] Очистка всех конфигураций и загрузка дефолтных настроек");

        // Очистка всех конфигураций после перезагрузки устройства
        clearAllConfigs();

        // Загрузка всех дефолтных конфигураций
        loadAllDefaultConfigs();

        logger->log(farm::log::Level::Info, "[Config] Настройки загружены успешно");
        return true;
    }

    bool ConfigManager::clearConfig(ConfigType type)
    {
        // Очистка конфигурации указанного типа
        auto& doc = getConfigDocument(type);
        doc.clear();
        logger->log(farm::log::Level::Debug, "[Config] Конфигурация %s очищена", getConfigPath(type));
        return saveConfig(type);
    }

    bool ConfigManager::clearAllConfigs()
    {
        bool success = true;

        success &= clearConfig(ConfigType::Data);
        success &= clearConfig(ConfigType::System);
        success &= clearConfig(ConfigType::Command);
        success &= clearConfig(ConfigType::Mqtt);

        return success;
    }

    bool ConfigManager::deleteConfig(ConfigType type)
    {
        const char* path = getConfigPath(type);
        logger->log(farm::log::Level::Warning, "[Config] Удаление конфигурации %s", path);
        return SPIFFS.remove(path);
    }

    bool ConfigManager::deleteAllConfigs()
    {
        bool success = true;

        success &= deleteConfig(ConfigType::Data);
        success &= deleteConfig(ConfigType::System);
        success &= deleteConfig(ConfigType::Command);
        success &= deleteConfig(ConfigType::Mqtt);

        return success;
    }
    
    // Загрузка JSON из файла
    bool ConfigManager::loadJsonFromFile(const char* path, JsonDocument& doc)
    {
        if (!SPIFFS.exists(path)) {
            logger->log(farm::log::Level::Warning, 
                      "[Config] Файл '%s' не существует", path);
            return false;
        }
        
        File file = SPIFFS.open(path, "r");
        if (!file) {
            logger->log(farm::log::Level::Error, 
                      "[Config] Не удалось открыть файл '%s'", path);
            return false;
        }
        
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        
        if (error) {
            logger->log(farm::log::Level::Error, 
                      "[Config] Ошибка десериализации JSON: %s", error.c_str());
            return false;
        }
        
        logger->log(farm::log::Level::Debug, 
                  "[Config] Файл '%s' успешно загружен", path);
        return true;
    }

    // Сохранение JSON в файл
    bool ConfigManager::saveJsonToFile(const char* path, const JsonDocument& doc)
    {
        File file = SPIFFS.open(path, "w");
        if (!file) {
            logger->log(farm::log::Level::Error, 
                      "[Config] Не удалось открыть файл '%s' для записи", path);
            return false;
        }
        
        if (serializeJson(doc, file) == 0) {
            logger->log(farm::log::Level::Error, 
                      "[Config] Ошибка сериализации JSON в файл '%s'", path);
            file.close();
            return false;
        }
        
        file.close();
        logger->log(farm::log::Level::Debug, 
                  "[Config] Файл '%s' успешно сохранен", path);
        return true;
    }

    // Получение пути к файлу конфигурации
    const char* ConfigManager::getConfigPath(ConfigType type) const
    {
        switch (type) {
            case ConfigType::Data:
                return paths::DATA_CONFIG;
            case ConfigType::System:
                return paths::SYSTEM_CONFIG;
            case ConfigType::Command:
                return paths::COMMAND_CONFIG;
            case ConfigType::Mqtt:
                return paths::MQTT_CONFIG;
            default:
                logger->log(farm::log::Level::Error, 
                          "[Config] Неизвестный тип конфигурации");
                return "";
        }
    }
    
    // Получение пути к дефолтному файлу конфигурации
    const char* ConfigManager::getDefaultConfigPath(ConfigType type) const
    {
        switch (type) {
            case ConfigType::Data:
                return paths::DEFAULT_DATA_CONFIG;
            case ConfigType::System:
                return paths::DEFAULT_SYSTEM_CONFIG;
            case ConfigType::Command:
                return paths::DEFAULT_COMMAND_CONFIG;
            case ConfigType::Mqtt:
                return paths::DEFAULT_MQTT_CONFIG;
            default:
                logger->log(farm::log::Level::Error, 
                          "[Config] Неизвестный тип конфигурации для дефолтного файла");
                return "";
        }
    }

    // Получение JSON документа для указанного типа
    // TODO: возвращаемое значение по дефолту нехорошее, но не критично, т.к. всегда передаем валидный тип
    JsonDocument& ConfigManager::getConfigDocument(ConfigType type)
    {
        switch (type) {
            case ConfigType::Data:
                return dataConfig;
            case ConfigType::System:
                return systemConfig;
            case ConfigType::Command:
                return commandConfig;
            case ConfigType::Mqtt:
                return mqttConfig;
            default:
                logger->log(farm::log::Level::Error, 
                          "[Config] Неизвестный тип конфигурации");
                return dataConfig; // Возвращаем dataConfig как значение по умолчанию
        }
    }

    // Получение константной ссылки на JSON документ для указанного типа
    // TODO: возвращаемое значение по дефолту нехорошее, но не критично, т.к. всегда передаем валидный тип
    const JsonDocument& ConfigManager::getConfigDocument(ConfigType type) const
    {
        switch (type) {
            case ConfigType::Data:
                return dataConfig;
            case ConfigType::System:
                return systemConfig;
            case ConfigType::Command:
                return commandConfig;
            case ConfigType::Mqtt:
                return mqttConfig;
            default:
                logger->log(farm::log::Level::Error, 
                          "[Config] Неизвестный тип конфигурации");
                return dataConfig; // Возвращаем dataConfig как значение по умолчанию
        }
    }
    
    // Загрузка дефолтной конфигурации
    bool ConfigManager::loadDefaultConfig(ConfigType type)
    {
        const char* defaultPath = getDefaultConfigPath(type);
        auto& doc = getConfigDocument(type);
        
        logger->log(farm::log::Level::Info, 
                  "[Config] Загрузка дефолтной конфигурации из '%s'", defaultPath);
        
        if (!SPIFFS.exists(defaultPath)) {
            logger->log(farm::log::Level::Error, 
                     "[Config] Дефолтный файл '%s' не существует", defaultPath);
            return false;
        }
        
        if (loadJsonFromFile(defaultPath, doc)) {
            // После загрузки дефолтной конфигурации, сохраняем её в основной файл
            const char* configPath = getConfigPath(type);
            logger->log(farm::log::Level::Debug, 
                      "[Config] Создание нового файла '%s' на основе дефолтной конфигурации", 
                      configPath);
            return saveJsonToFile(configPath, doc);
        }
        
        return false;
    }

    bool ConfigManager::loadAllDefaultConfigs()
    {
        bool success = true;

        success &= loadDefaultConfig(ConfigType::Data);
        success &= loadDefaultConfig(ConfigType::System);
        success &= loadDefaultConfig(ConfigType::Command);
        success &= loadDefaultConfig(ConfigType::Mqtt);

        return success;
    }

    // Загрузка конфигурации
    bool ConfigManager::loadConfig(ConfigType type)
    {
        const char* path = getConfigPath(type);
        auto& doc = getConfigDocument(type);
        
        // Сначала очищаем документ
        doc.clear();
        
        // Если файл не существует, пробуем загрузить дефолтную конфигурацию
        if (!SPIFFS.exists(path)) {
            logger->log(farm::log::Level::Warning, 
                      "[Config] Файл '%s' не существует, попытка загрузки дефолтной конфигурации", path);
            return loadDefaultConfig(type);
        }
        
        return loadJsonFromFile(path, doc);
    }

    // Сохранение конфигурации
    bool ConfigManager::saveConfig(ConfigType type)
    {
        const char* path = getConfigPath(type);
        const auto& doc = getConfigDocument(type);
        
        return saveJsonToFile(path, doc);
    }

    // Загрузка всех конфигураций
    bool ConfigManager::loadAllConfigs()
    {
        bool success = true;
        
        success &= loadConfig(ConfigType::Data);
        success &= loadConfig(ConfigType::System);
        success &= loadConfig(ConfigType::Command);
        success &= loadConfig(ConfigType::Mqtt);
        
        logger->log(farm::log::Level::Info, 
                  "[Config] Загрузка всех конфигураций %s", 
                  success ? "успешна" : "завершилась с ошибками");
        
        return success;
    }

    // Сохранение всех конфигураций
    bool ConfigManager::saveAllConfigs()
    {
        bool success = true;
        
        success &= saveConfig(ConfigType::Data);
        success &= saveConfig(ConfigType::System);
        success &= saveConfig(ConfigType::Command);
        success &= saveConfig(ConfigType::Mqtt);
        
        logger->log(farm::log::Level::Info, 
                  "[Config] Сохранение всех конфигураций %s", 
                  success ? "успешно" : "завершилось с ошибками");
        
        return success;
    }

    // Проверка существования ключа
    bool ConfigManager::hasKey(ConfigType type, const char* key) const
    {
        const auto& doc = getConfigDocument(type);
        // В ArduinoJson 7 вместо containsKey() рекомендуется использовать is<T>
        // Проверяем, существует ли ключ (не null)
        return !doc[key].isNull();
    }

    // Вывод текущей конфигурации для отладки
    void ConfigManager::printConfig(ConfigType type) const
    {
        const auto& doc = getConfigDocument(type);
        
        // Сначала вычислить размер необходимого буфера
        size_t jsonSize = measureJsonPretty(doc);
        // Затем создать строку нужного размера
        String output;
        output.reserve(jsonSize + 1); // +1 для нулевого терминатора
        // И только потом сериализовать
        serializeJsonPretty(doc, output);
        
        // Проверяем, что размер вывода не превышает максимально допустимый размер буфера лога
        // и что строка не содержит обрывков UTF-8 символов на границе
        // -58, т.к. в логе будет ещё "[Config] Текущая конфигурация %s:\n%s"
        // TODO: сделать по-человечески
        if (output.length() <= farm::log::constants::LOG_BUFFER_SIZE - 58)
        {
            logger->log(farm::log::Level::Info, 
                      "[Config] Текущая конфигурация %s:\n%s", 
                      getConfigPath(type), output.c_str());
        }
        else 
        {
            if (logger->getLevel() >= farm::log::Level::Info)
            {
                logger->log(farm::log::Level::Warning, 
                        "[Config] Текущая конфигурация %s слишком большая для вывода в лог", 
                        getConfigPath(type));
                Serial.println(output);
            }
        }
    }

    // Получение JSON строки по типу конфигурации
    String ConfigManager::getConfigJson(ConfigType type) const
    {
        const auto& doc = getConfigDocument(type);
        
        // Сначала вычислить размер необходимого буфера
        size_t jsonSize = measureJsonPretty(doc);
        // Затем создать строку нужного размера
        String jsonString;
        jsonString.reserve(jsonSize + 1); // +1 для нулевого терминатора
        // И только потом сериализовать
        serializeJson(doc, jsonString);
        
        return jsonString;
    }

    // Перезапись JSON файла из строки
    bool ConfigManager::updateFromJson(ConfigType type, const String& jsonString)
    {
        // Создаем временный JSON документ для десериализации входящих данных
        JsonDocument tempDoc;
        
        // Десериализация входящей строки во временный документ
        DeserializationError error = deserializeJson(tempDoc, jsonString);
        if (error) 
        {
            logger->log(farm::log::Level::Error, 
                      "[Config] Ошибка десериализации JSON: %s", error.c_str());
            return false;
        }
        
        // Получаем ссылку на целевой документ
        auto& targetDoc = getConfigDocument(type);
        
        // Рекурсивная функция для слияния JSON объектов с любой глубиной вложенности
        std::function<void(JsonVariant, JsonVariant)> mergeJson = 
            [&mergeJson](JsonVariant src, JsonVariant dest) 
            {
                // Если источник не является объектом, нечего сливать
                if (!src.is<JsonObject>()) 
                {
                    return;
                }
                
                // Получаем объект-источник
                JsonObject srcObj = src.as<JsonObject>();
                
                // Если целевой объект не существует, создаем его
                if (!dest.is<JsonObject>()) 
                {
                    dest.to<JsonObject>();
                }
                
                // Получаем целевой объект
                JsonObject destObj = dest.as<JsonObject>();
                
                // Обходим все ключи в источнике
                for (JsonPair kv : srcObj) 
                {
                    const char* key = kv.key().c_str();
                    JsonVariant srcValue = kv.value();
                    
                    // Если значение в источнике - объект, рекурсивно сливаем
                    if (srcValue.is<JsonObject>()) 
                    {
                        // Рекурсивно вызываем слияние для вложенных объектов
                        mergeJson(srcValue, destObj[key]);
                    } 
                    // Если значение в источнике - массив, заменяем целиком
                    else if (srcValue.is<JsonArray>()) 
                    {
                        destObj[key] = srcValue;
                    } 
                    // Для всех остальных типов просто копируем значение
                    else 
                    {
                        destObj[key] = srcValue;
                    }
                }
            };
        
        // Запускаем рекурсивное слияние
        mergeJson(tempDoc, targetDoc);
        
        logger->log(farm::log::Level::Info, 
                  "[Config] Конфигурация %s обновлена из JSON", 
                  getConfigPath(type));
        
        return true;
    }

    // Проверка, настроен ли MQTT
    bool ConfigManager::isMqttConfigured() const
    {
        // MQTT настроен, если указаны хост и порт
        return hasKey(ConfigType::Mqtt, "host") && 
               hasKey(ConfigType::Mqtt, "port") && 
               mqttConfig["host"].as<String>().length() > 0 && 
               mqttConfig["port"].as<int16_t>() >= 0;
    }
    
    // Получить топик для данных
    String ConfigManager::getDataTopic() const
    {
        String deviceId = hasKey(ConfigType::Mqtt, "deviceId") 
            ? mqttConfig["deviceId"].as<String>()
            : mqtt::DEFAULT_DEVICE_ID;
            
        return deviceId + mqtt::DATA_SUFFIX;
    }
    
    // Получить топик для конфигурации
    String ConfigManager::getConfigTopic() const
    {
        String deviceId = hasKey(ConfigType::Mqtt, "deviceId") 
            ? mqttConfig["deviceId"].as<String>()
            : mqtt::DEFAULT_DEVICE_ID;
            
        return deviceId + mqtt::CONFIG_SUFFIX;
    }
    
    // Получить топик для команд
    String ConfigManager::getCommandTopic() const
    {
        String deviceId = hasKey(ConfigType::Mqtt, "deviceId") 
            ? mqttConfig["deviceId"].as<String>()
            : mqtt::DEFAULT_DEVICE_ID;
            
        return deviceId + mqtt::COMMAND_SUFFIX;
    }
}
