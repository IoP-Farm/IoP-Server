#include "config/config_manager.h"

// TODO: сейчас команда для начальной загрузки файлов конфигурации в память: pio run --target uploadfs

/*
    Для загрузки файлов в SPIFFS: pio run --target uploadfs
    Для скачивания содержимого SPIFFS можно использовать: pio run --target downloadfs
*/

namespace farm::config
{
    // Использование сокращений для более компактного кода
    using farm::log::Level;
    using farm::log::LoggerFactory;
    
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
            this->logger = LoggerFactory::createSerialLogger(Level::Info);
        } 
        else 
        {
            this->logger = logger;
        }
        
        this->logger->log(Level::Info, "[Config] Инициализация ConfigManager");
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
        logger->log(Level::Debug, "[Config] Освобождение памяти ConfigManager");

        // Освобождаем все JSON документы
        dataConfig.clear();
        systemConfig.clear();
        commandConfig.clear();
        mqttConfig.clear();
    }

    // Инициализация SPIFFS
    bool ConfigManager::initialize()
    {
        logger->log(Level::Info, "[Config] Инициализация SPIFFS");
        
        if (!SPIFFS.begin(true)) {
            logger->log(Level::Error, "[Config] Ошибка инициализации SPIFFS");
            return false;
        }
        
        // Проверяем наличие всех необходимых файлов
        bool allFilesExist = true;
        
        // Проверка дефолтных файлов конфигурации
        if (!SPIFFS.exists(paths::DEFAULT_DATA_CONFIG)) {
            logger->log(Level::Error, 
                       "[Config] Отсутствует файл: %s", 
                       paths::DEFAULT_DATA_CONFIG);
            allFilesExist = false;
        }
        
        if (!SPIFFS.exists(paths::DEFAULT_SYSTEM_CONFIG)) {
            logger->log(Level::Error, 
                       "[Config] Отсутствует файл: %s", 
                       paths::DEFAULT_SYSTEM_CONFIG);
            allFilesExist = false;
        }
        
        if (!SPIFFS.exists(paths::DEFAULT_COMMAND_CONFIG)) {
            logger->log(Level::Error, 
                       "[Config] Отсутствует файл: %s", 
                       paths::DEFAULT_COMMAND_CONFIG);
            allFilesExist = false;
        }
        
        if (!SPIFFS.exists(paths::DEFAULT_MQTT_CONFIG)) {
            logger->log(Level::Error, 
                       "[Config] Отсутствует файл: %s", 
                       paths::DEFAULT_MQTT_CONFIG);
            allFilesExist = false;
        }

        if (!allFilesExist) {
            logger->log(Level::Error, 
                       "[Config] Отсутствуют необходимые файлы. "
                       "Убедитесь, что файлы загружены в SPIFFS с помощью команды: "
                       "pio run --target uploadfs");
            return false;
        }

        logger->log(Level::Info, "[Config] Загрузка стандартных настроек");

        // Очистка всех конфигураций после перезагрузки устройства
        clearAllConfigs();

        // Загрузка всех дефолтных конфигураций
        loadAllDefaultConfigs();

        return true;
    }

    bool ConfigManager::clearConfig(ConfigType type)
    {
        // Очистка конфигурации указанного типа
        auto& doc = getConfigDocument(type);
        doc.clear();
        logger->log(Level::Debug, "[Config] %s очищен", getConfigPath(type));
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
        logger->log(Level::Warning, "[Config] Удаление %s", path);
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
            logger->log(Level::Warning, 
                      "[Config] Файл '%s' не существует", path);
            return false;
        }
        
        File file = SPIFFS.open(path, "r");
        if (!file) {
            logger->log(Level::Error, 
                      "[Config] Не удалось открыть файл '%s'", path);
            return false;
        }
        
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        
        if (error) {
            logger->log(Level::Error, 
                      "[Config] Ошибка десериализации JSON: %s", error.c_str());
            return false;
        }

        return true;
    }

    // Сохранение JSON в файл
    bool ConfigManager::saveJsonToFile(const char* path, const JsonDocument& doc)
    {
        File file = SPIFFS.open(path, "w");
        if (!file) {
            logger->log(Level::Error, 
                      "[Config] Не удалось открыть файл '%s' для записи", path);
            return false;
        }
        
        if (serializeJson(doc, file) == 0) {
            logger->log(Level::Error, 
                      "[Config] Ошибка сериализации JSON в файл '%s'", path);
            file.close();
            return false;
        }
        
        file.close();
        logger->log(Level::Debug, 
                  "[Config] '%s' сохранен в SPIFFS", path);
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
                logger->log(Level::Error, 
                          "[Config] Неизвестный тип настроек");
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
                logger->log(Level::Error, 
                          "[Config] Неизвестный тип стандартных настроек");
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
                logger->log(Level::Error, 
                          "[Config] Неизвестный тип настроек");
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
                logger->log(Level::Error, 
                          "[Config] Неизвестный тип настроек");
                return dataConfig; // Возвращаем dataConfig как значение по умолчанию
        }
    }
    
    // Загрузка дефолтной конфигурации
    bool ConfigManager::loadDefaultConfig(ConfigType type)
    {
        const char* defaultPath = getDefaultConfigPath(type);
        auto& doc               = getConfigDocument(type);
        
        logger->log(Level::Info, 
                  "[Config] Загрузка стандартных настроек из '%s'", defaultPath);
        
        if (!SPIFFS.exists(defaultPath)) {
            logger->log(Level::Error, 
                     "[Config] '%s' отсутствует", defaultPath);
            return false;
        }
        
        if (loadJsonFromFile(defaultPath, doc)) {
            // После загрузки дефолтной конфигурации, сохраняем её в основной файл
            const char* configPath = getConfigPath(type);
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
        auto& doc       = getConfigDocument(type);
        
        // Сначала очищаем документ
        doc.clear();
        
        // Если файл не существует, пробуем загрузить дефолтную конфигурацию
        if (!SPIFFS.exists(path)) {
            logger->log(Level::Warning, 
                      "[Config] '%s' отсутствует, попытка загрузки стандартных настроек", path);
            return loadDefaultConfig(type);
        }
        
        return loadJsonFromFile(path, doc);
    }

    // Сохранение конфигурации
    bool ConfigManager::saveConfig(ConfigType type)
    {
        const char* path   = getConfigPath(type);
        const auto& doc    = getConfigDocument(type);
        
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
        
        logger->log(Level::Info, 
                  "[Config] Сохранение всех настроек %s", 
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
            logger->log(Level::Info, 
                      "[Config] Текущий файл %s:\n%s", 
                      getConfigPath(type), output.c_str());
        }
        else 
        {
            if (logger->getLevel() >= Level::Info)
            {
                logger->log(Level::Warning, 
                        "[Config] %s слишком большой для вывода в лог", 
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
            logger->log(Level::Error, 
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
                    const char* key       = kv.key().c_str();
                    JsonVariant srcValue  = kv.value();
                    
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
        
        logger->log(Level::Info, 
                  "[Config] %s обновлен из JSON-строки", 
                  getConfigPath(type));
        
        return true;
    }
}
