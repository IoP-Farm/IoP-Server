#include "network/mqtt_manager.h"
#include <WiFi.h>

namespace farm::net
{
    // Используем константы MQTT и сокращаем пространства имен для удобства
    using namespace farm::config::mqtt;
    using farm::config::ConfigType;
    using farm::config::CommandCode;
    using farm::log::Level;
    
    // Инициализация статического экземпляра
    std::shared_ptr<MQTTManager> MQTTManager::instance = nullptr;
    
    // Конструктор
    MQTTManager::MQTTManager(std::shared_ptr<farm::log::ILogger> logger)
    {
        // Если логгер не передан, создаем новый с помощью фабрики
        if (!logger) 
        {
            this->logger = farm::log::LoggerFactory::createSerialLogger(Level::Info);
        } 
        else 
        {
            this->logger = logger;
        }
        
        // Получаем экземпляр ConfigManager
        configManager = farm::config::ConfigManager::getInstance(this->logger);
        
        logger->log(Level::Info, "[MQTT] Инициализация MQTT");
    }
    
    // Получение экземпляра синглтона
    std::shared_ptr<MQTTManager> MQTTManager::getInstance(std::shared_ptr<farm::log::ILogger> logger)
    {
        if (instance == nullptr) 
        {
            // Используем явное создание вместо make_shared, т.к. конструктор приватный
            instance = std::shared_ptr<MQTTManager>(new MQTTManager(logger));
        }
        return instance;
    }
    
    // Деструктор
    MQTTManager::~MQTTManager()
    {
        logger->log(Level::Debug, "[MQTT] Освобождение ресурсов MQTT Manager");
        
        // Отключаемся от MQTT сервера
        if (mqttClient.connected()) 
        {
            mqttClient.disconnect();
        }
    }
    
    // Инициализация MQTT и настройка колбэков
    bool MQTTManager::initialize()
    {
        logger->log(Level::Info, "[MQTT] Инициализация MQTT");
        
        // Проверяем настройки MQTT
        if (!isMqttConfigured()) 
        {
            logger->log(Level::Warning, 
                     "[MQTT] MQTT не настроен. Необходимо задать Host и Port");
            isInitialized = false;
            return false;
        }
        
        // Настройка MQTT клиента
        setupMQTTClient();


        // Устанавливаем флаг успешной инициализации
        isInitialized = true;

        return true;
    }
    
    // Настройка MQTT клиента и колбэков
    bool MQTTManager::setupMQTTClient()
    {
        // Получаем настройки MQTT из конфигурации
        String host     = configManager->getValue<String>(ConfigType::Mqtt,  "host");
        int16_t port    = configManager->getValue<int16_t>(ConfigType::Mqtt, "port");
        String deviceId = configManager->getValue<String>(ConfigType::Mqtt,  "deviceId");
        
        // Сохраняем информацию о сервере
        serverDomain = host;
        serverPort   = port;

        if (host.length() == 0 || port < 0 || deviceId.length() == 0)
        {
            logger->log(Level::Error, 
                      "[MQTT] Некорректные настройки MQTT: Host=%s, Port=%d, DeviceId=%s", 
                      host.c_str(), port, deviceId.c_str());
            return false;
        }
        
        logger->log(Level::Debug, 
                  "[MQTT] Настраиваем MQTT: Server=%s, Port=%d, DeviceId=%s", 
                  serverDomain.c_str(), serverPort, deviceId.c_str());
        
        // Преобразуем строку IP-адреса в объект IPAddress
        IPAddress serverIP;
        if (serverIP.fromString(host)) 
        {
            // Если удалось преобразовать строку в IP-адрес
            logger->log(Level::Debug, 
                      "[MQTT] Подключение к серверу будет производиться по IP");
            
            // Установка IP-адреса и порта
            mqttClient.setServer(serverIP, serverPort);
        } 
        else 
        {
            // Если это доменное имя или неверный формат IP
            logger->log(Level::Warning, 
                      "[MQTT] Подключение к серверу будет производиться по имени хоста");
            
            // Установка параметров сервера по строке хоста
            mqttClient.setServer(serverDomain.c_str(), serverPort);
        }
        
        // Установка clientId
        if (deviceId.length() > 0)  
        {
            strncpy(deviceIdBuffer, deviceId.c_str(), deviceId.length());  
            deviceIdBuffer[deviceId.length()] = '\0';
            mqttClient.setClientId(deviceIdBuffer);
            logger->log(Level::Debug, "[MQTT] Установлен clientId: %s", deviceIdBuffer);
        }
        else
        {
            logger->log(Level::Error, "[MQTT] Нет deviceId при настройке MQTT");
            return false;
        }
        
        // Настройка колбэков
        mqttClient.onConnect([this](bool sessionPresent) {
            this->onMqttConnect(sessionPresent);
        });
        
        mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
            this->onMqttDisconnect(reason);
        });
        
        mqttClient.onSubscribe([this](uint16_t packetId, uint8_t qos) {
            this->onMqttSubscribe(packetId, qos);
        });
        
        mqttClient.onUnsubscribe([this](uint16_t packetId) {
            this->onMqttUnsubscribe(packetId);
        });
        
        mqttClient.onMessage([this](char* topic, char* payload, 
                                  AsyncMqttClientMessageProperties properties,
                                  size_t len, size_t index, size_t total) {
            this->onMqttMessage(topic, payload, properties, len, index, total);
        });
        
        mqttClient.onPublish([this](uint16_t packetId) {
            this->onMqttPublish(packetId);
        });

        logger->log(Level::Debug, "[MQTT] MQTT Callbacks настроены");

        return true;
    }
    
    // Проверка, настроен ли MQTT
    bool MQTTManager::isMqttConfigured() const
    {
        // MQTT настроен, если указаны хост и порт
        return configManager->hasKey(ConfigType::Mqtt, "host") && 
               configManager->hasKey(ConfigType::Mqtt, "port") && 
               configManager->getValue<String>(ConfigType::Mqtt, "host").length() > 0 && 
               configManager->getValue<int16_t>(ConfigType::Mqtt, "port") >= 0;
    }
    
    // Получить топик MQTT в зависимости от типа
    String MQTTManager::getMqttTopic(ConfigType type) const
    {
        String deviceId = configManager->hasKey(ConfigType::Mqtt, "deviceId") 
            ? configManager->getValue<String>(ConfigType::Mqtt, "deviceId")
            : DEFAULT_DEVICE_ID;
            
        switch (type)
        {
            case ConfigType::Data:
                return "/" + deviceId + DATA_SUFFIX;
                
            case ConfigType::System:
                return "/" + deviceId + CONFIG_SUFFIX;
                
            case ConfigType::Command:
                return "/" + deviceId + COMMAND_SUFFIX;
                
            default:
                logger->log(Level::Error, 
                          "[MQTT] Запрошен топик для неподдерживаемого типа настроек");
                return "/" + deviceId + "/unknown";
        }
    }
    
    // Обработчик подключения к MQTT
    void MQTTManager::onMqttConnect(bool sessionPresent)
    {
        isConnected      = true;
        isConnecting     = false;
        reconnectAttempts = 0;
        
        logger->log(Level::Info, 
                  "[MQTT] Успешное подключение к серверу %s:%d", 
                  serverDomain.c_str(), 
                  serverPort);
        
        subscribeToAllTopics();
    }
    
    // Обработчик отключения от MQTT
    void MQTTManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
    {
        isConnected  = false;
        isConnecting = false;
        
        const char* reasonStr;
        switch (reason) {
            case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
                reasonStr = "TCP соединение разорвано";
                break;
            case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
                reasonStr = "Неприемлемая версия протокола";
                break;
            case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
                reasonStr = "Идентификатор отклонен";
                break;
            case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
                reasonStr = "Сервер недоступен";
                break;
            case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
                reasonStr = "Неверный формат учетных данных";
                break;
            case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
                reasonStr = "MQTT сервер отклонил учетные данные";
                break;
            case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE:
                reasonStr = "Недостаточно памяти";
                break;
            case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT:
                reasonStr = "Неверный отпечаток TLS";
                break;
            default:
                reasonStr = "Неизвестная причина";
                break;
        }
        
        logger->log(Level::Warning, 
                  "[MQTT] Отключено от сервера. Причина: %s", reasonStr);
    }
    
    // Обработчик успешной подписки
    void MQTTManager::onMqttSubscribe(uint16_t packetId, uint8_t qos)
    {
    }
    
    // Обработчик успешной отписки
    void MQTTManager::onMqttUnsubscribe(uint16_t packetId)
    {
        logger->log(Level::Debug, 
                  "[MQTT] Успешная отписка от топика, packetId=%d", packetId);
    }
    
    // Обработчик полученного сообщения
    void MQTTManager::onMqttMessage(char* topic, char* payload, 
                                 AsyncMqttClientMessageProperties properties,
                                 size_t len, size_t index, size_t total)
    {
        // Создаем временный буфер для сообщения (добавляем нулевой символ в конец)
        char* message = new char[len + 1];
        memcpy(message, payload, len);
        message[len] = '\0';
        
        logger->log(Level::Debug, 
                  "[MQTT] Получено сообщение в топик '%s'", topic);
        
        // Обработка сообщений в зависимости от топика
        String topicStr(topic);
        
        // Получаем топики из конфигурации
        String configTopic  = getMqttTopic(ConfigType::System);
        String commandTopic = getMqttTopic(ConfigType::Command);
        
        // Обработка сообщений конфигурации
        if (topicStr == configTopic) 
        {
            logger->log(Level::Info, 
                      "[MQTT] Обновление Config");
            
            // Обновляем системную конфигурацию
            bool updated = configManager->updateFromJson(ConfigType::System, message);
            if (updated) 
            {
                configManager->saveConfig(ConfigType::System);
                configManager->printConfig(ConfigType::System);
            }
            else 
            {
                logger->log(Level::Error, 
                          "[MQTT] Ошибка обновления Config");
            }
        }
        // Обработка команд
        else if (topicStr == commandTopic) 
        {
            logger->log(Level::Info, 
                      "[MQTT] Обработка полученной команды");
            
            // Обновляем команду
            bool updated = configManager->updateFromJson(ConfigType::Command, message);
            if (updated) 
            {
                configManager->saveConfig(ConfigType::Command);
                configManager->printConfig(ConfigType::Command);
                
                // Получаем код команды
                int commandInt = configManager->getValue<int>(ConfigType::Command, "command");
                
                // Приводим к CommandCode и обрабатываем
                CommandCode command = static_cast<CommandCode>(commandInt);
                handleCommand(command);
            }
            else 
            {
                logger->log(Level::Error, 
                          "[MQTT] Ошибка обработки полученной команды");
            }
        }
        
        // Освобождаем память
        delete[] message;
    }
    
    // Обработчик успешной публикации
    void MQTTManager::onMqttPublish(uint16_t packetId)
    {
    }
    
    // Подписка на конкретный топик
    uint16_t MQTTManager::subscribeToTopic(const String& topic, uint8_t qos)
    {
        if (!isClientConnected()) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Не удалось подписаться на топик: нет соединения с MQTT сервером");
            return 0;
        }
        
        // Отправляем запрос на подписку
        uint16_t packetId = mqttClient.subscribe(topic.c_str(), qos);
        
        if (packetId > 0)
        {
            logger->log(Level::Info, 
                      "[MQTT] Подписка на топик '%s' с QoS=%d", 
                      topic.c_str(), qos);
        }
        else 
        {
            logger->log(Level::Error, 
                      "[MQTT] Ошибка при подписке на топик '%s'", 
                      topic.c_str());
        }
        
        return packetId;
    }
    
    // Подписка на все стандартные топики (публичный метод)
    bool MQTTManager::subscribeToAllTopics(uint8_t qos)
    {
        if (!isClientConnected()) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Не удалось подписаться на топики: нет соединения с MQTT сервером");
            return false;
        }
        
        // Получаем топики из конфигурации
        String configTopic  = getMqttTopic(ConfigType::System);
        String commandTopic = getMqttTopic(ConfigType::Command);
        
        // Подписываемся на топики конфигурации и команд
        uint16_t packetIdConfig  = subscribeToTopic(configTopic, qos);
        uint16_t packetIdCommand = subscribeToTopic(commandTopic, qos);
        
        // Проверяем успешность подписки
        bool success = (packetIdConfig > 0) && (packetIdCommand > 0);
        
        return success;
    }
    
    // Обработка полученной команды
    void MQTTManager::handleCommand(const CommandCode& command)
    {
        logger->log(Level::Info, 
                  "[MQTT] Обработка команды %d", static_cast<int>(command));
        
        // Обработка разных команд
        switch (command) 
        {
            case CommandCode::ESP_RESTART:
                logger->log(Level::Warning, 
                          "[MQTT] Получена команда на перезагрузку ESP");
                // Добавим задержку, чтобы ответ успел уйти
                delay(200);
                ESP.restart();
                break;
                
            case CommandCode::PUMP_ON:
                logger->log(Level::Info, 
                          "[MQTT] Команда включения насоса (заглушка)");
                // TODO: Реализовать включение насоса
                break;
                
            case CommandCode::PUMP_OFF:
                logger->log(Level::Info, 
                          "[MQTT] Команда выключения насоса (заглушка)");
                // TODO: Реализовать выключение насоса
                break;
                
            case CommandCode::GROWLIGHT_ON:
                logger->log(Level::Info, 
                          "[MQTT] Команда включения фитолампы (заглушка)");
                // TODO: Реализовать включение фитолампы
                break;
                
            case CommandCode::GROWLIGHT_OFF:
                logger->log(Level::Info, 
                          "[MQTT] Команда выключения фитолампы (заглушка)");
                // TODO: Реализовать выключение фитолампы
                break;
                
            case CommandCode::HEATLAMP_ON:
                logger->log(Level::Info, 
                          "[MQTT] Команда включения лампы нагрева (заглушка)");
                // TODO: Реализовать включение лампы нагрева
                break;
                
            case CommandCode::HEATLAMP_OFF:
                logger->log(Level::Info, 
                          "[MQTT] Команда выключения лампы нагрева (заглушка)");
                // TODO: Реализовать выключение лампы нагрева
                break;
                
            default:
                logger->log(Level::Warning, 
                          "[MQTT] Неизвестная команда: %d", static_cast<int>(command));
                break;
        }
    }
    
    // Поддержание MQTT соединения
    void MQTTManager::maintainConnection()
    {
        // Периодическая проверка MQTT соединения
        unsigned long currentMillis = millis();
        if (currentMillis - lastCheckTime >= CHECK_INTERVAL) 
        {
            lastCheckTime = currentMillis;
            
            // Новая проверка: если MQTT не инициализировано, но настройки появились
            if (!isInitialized) 
            {
                if (isMqttConfigured())
                {
                    logger->log(Level::Info, 
                              "[MQTT] Обнаружены новые настройки подключения к MQTT серверу");
                    initialize();
                }
                return;
            }
            
            // Если не подключены к MQTT, но подключены к WiFi
            if (!isClientConnected() && WiFi.isConnected() && isMqttConfigured()) 
            {
                // Не пытаемся переподключаться слишком часто и не в процессе подключения
                if (!isConnecting && (currentMillis - lastReconnectTime >= RECONNECT_RETRY_INTERVAL)) 
                {
                    lastReconnectTime = currentMillis;
                    
                    logger->log(Level::Info, "[MQTT] Попытка подключения к %s:%d", serverDomain.c_str(), serverPort);
                    
                    isConnecting = true;
                    
                    mqttClient.connect();
                }
            }
        }
    }
    
    // Публикация данных в MQTT
    bool MQTTManager::publishData(uint8_t qos, bool retain)
    {
        if (!isClientConnected()) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Не удалось опубликовать данные: нет соединения с MQTT сервером");
            return false;
        }
        
        // Получаем топик для данных
        String dataTopic = getMqttTopic(ConfigType::Data);
        
        // Получаем данные из конфигурации
        String jsonData = configManager->getConfigJson(ConfigType::Data);
        
        // Публикуем данные (QoS=1, retain=true)
        uint16_t packetId = mqttClient.publish(dataTopic.c_str(), qos, retain, jsonData.c_str());
        
        if (packetId > 0)
        {
            logger->log(Level::Info, 
                      "[MQTT] Данные #%d опубликованы в топик '%s'", 
                      packetId, dataTopic.c_str());
        }
        else
        {
            logger->log(Level::Error, 
                      "[MQTT] Ошибка при публикации данных в топик '%s'", 
                      dataTopic.c_str());
        }
        
        return packetId > 0;
    }
    
    // Отписка от топика
    uint16_t MQTTManager::unsubscribeFromTopic(const String& topic)
    {
        if (!isClientConnected()) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Не удалось отписаться от топика: нет соединения с MQTT сервером");
            return 0;
        }
        
        // Отправляем запрос на отписку
        uint16_t packetId = mqttClient.unsubscribe(topic.c_str());
        
        if (packetId > 0)
        {
            logger->log(Level::Info, 
                      "[MQTT] Запрос на отписку от топика '%s' отправлен (packetId=%d)", 
                      topic.c_str(), packetId);
        }
        else 
        {
            logger->log(Level::Error, 
                      "[MQTT] Ошибка при отправке запроса на отписку от топика '%s'", 
                      topic.c_str());
        }
        
        return packetId;
    }
    
    // Отписка от всех подписанных топиков
    bool MQTTManager::unsubscribeFromAllTopics()
    {
        if (!isClientConnected()) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Не удалось отписаться от топиков: нет соединения с MQTT сервером");
            return false;
        }
        
        // Отписываемся от топиков конфигурации и команд
        String configTopic  = getMqttTopic(ConfigType::System);
        String commandTopic = getMqttTopic(ConfigType::Command);
        
        uint16_t packetIdConfig  = unsubscribeFromTopic(configTopic);
        uint16_t packetIdCommand = unsubscribeFromTopic(commandTopic);
        
        // Проверяем успешность отправки запросов
        bool success = (packetIdConfig > 0) && (packetIdCommand > 0);
        
        logger->log(Level::Info, 
                  "[MQTT] Отписка от всех топиков %s", 
                  success ? "выполнена" : "завершилась с ошибками");
        
        return success;
    }
    
    // Проверка состояния подключения
    bool MQTTManager::isClientConnected() const
    {
        return isConnected && mqttClient.connected();
    }
    
    // Установка хоста MQTT
    bool MQTTManager::setMqttHost(const String& host)
    {
        // Проверяем, что хост не пустой
        if (host.length() == 0) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Невозможно установить пустой хост");
            return false;
        }
        
        // Сохраняем хост в конфигурации
        configManager->setValue<String>(ConfigType::Mqtt, "host", host);
        bool saved = configManager->saveConfig(ConfigType::Mqtt);
        
        if (saved) 
        {
            logger->log(Level::Info, 
                      "[MQTT] Установлен новый хост: %s", host.c_str());
            
            // Если это изменение настроек, переустанавливаем флаг инициализации
            if (isInitialized && host != serverDomain) 
            {
                isInitialized = false;
            }
        } 
        else 
        {
            logger->log(Level::Error, 
                      "[MQTT] Не удалось сохранить хост");
        }
        
        return saved;
    }
    
    // Установка порта MQTT
    bool MQTTManager::setMqttPort(uint16_t port)
    {
        // Проверяем валидность порта (0 обычно недопустим, стандартные порты MQTT: 1883 или 8883 для TLS)
        if (port <= 0) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Невозможно установить данный порта");
            return false;
        }
        
        // Сохраняем порт в конфигурации
        configManager->setValue<int16_t>(ConfigType::Mqtt, "port", port);
        bool saved = configManager->saveConfig(ConfigType::Mqtt);
        
        if (saved) 
        {
            logger->log(Level::Info, 
                      "[MQTT] Установлен новый порт: %d", port);
            
            // Если это изменение настроек, переустанавливаем флаг инициализации
            if (isInitialized && port != serverPort) 
            {
                isInitialized = false;
            }
        } 
        else 
        {
            logger->log(Level::Error, 
                      "[MQTT] Не удалось сохранить порт");
        }
        
        return saved;
    }
    
    // Установка deviceId для MQTT
    bool MQTTManager::setMqttDeviceId(const String& deviceId)
    {
        // Проверяем, что deviceId не пустой
        if (deviceId.length() == 0) 
        {
            logger->log(Level::Warning, 
                      "[MQTT] Невозможно установить пустой deviceId");
            return false;
        }
        
        // Сохраняем deviceId в конфигурации
        configManager->setValue<String>(ConfigType::Mqtt, "deviceId", deviceId);
        bool saved = configManager->saveConfig(ConfigType::Mqtt);
        
        if (saved) 
        {
            logger->log(Level::Info, 
                      "[MQTT] Установлен новый deviceId: %s", deviceId.c_str());
            
            // Если MQTT уже инициализирован, нужно его переинициализировать
            if (isInitialized) 
            {
                isInitialized = false;
            }
        } 
        else 
        {
            logger->log(Level::Error, 
                      "[MQTT] Не удалось сохранить deviceId");
        }
        
        return saved;
    }

    // Получение текущего хоста MQTT
    String MQTTManager::getMqttHost() const
    {
        return serverDomain;
    }
    
    // Получение текущего порта MQTT
    uint16_t MQTTManager::getMqttPort() const
    {
        return serverPort;
    }
    
    // Получение текущего deviceId MQTT
    String MQTTManager::getMqttDeviceId() const
    {
        return configManager->getValue<String>(ConfigType::Mqtt, "deviceId");
    }
    
    // Применение всех настроек MQTT одновременно
    bool MQTTManager::applyMqttSettings(const String& host, uint16_t port, const String& deviceId)
    {
        bool success = true;
        
        success &= setMqttHost(host);
        success &= setMqttPort(port);   
        success &= setMqttDeviceId(deviceId);
        
        // Если успешно обновили настройки и у нас есть все необходимое для подключения
        if (success && isMqttConfigured()) 
        {
            // Если уже были инициализированы, но параметры изменились
            if (isInitialized && 
                (serverDomain != host || serverPort != port || 
                 configManager->getValue<String>(ConfigType::Mqtt, "deviceId") != deviceId))
            {
                // Отключаемся от текущего сервера, если были подключены
                if (mqttClient.connected()) 
                {
                    mqttClient.disconnect();
                }
                
                isInitialized = false;
                logger->log(Level::Info, 
                          "[MQTT] Настройки изменены, требуется переинициализация");
            }
            
            // Если не инициализированы, запускаем инициализацию
            if (!isInitialized) 
            {
                logger->log(Level::Info, 
                          "[MQTT] Применяем новые настройки MQTT");
                initialize();
            }
        }
        
        return success;
    }
}
