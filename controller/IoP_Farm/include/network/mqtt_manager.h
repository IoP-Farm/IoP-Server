#pragma once

#include <AsyncMqttClient.h>
#include "utils/logger_factory.h"
#include "config/config_manager.h"
#include "config/constants.h"
#include <memory>

namespace farm::net
{
    // Менеджер MQTT соединения - синглтон с использованием std::shared_ptr
    class MQTTManager
    {
    private:
        // Приватный конструктор (паттерн Синглтон)
        explicit MQTTManager(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Статический экземпляр как shared_ptr
        static std::shared_ptr<MQTTManager> instance;
        
        // Логгер
        std::shared_ptr<farm::log::ILogger> logger;
        
        // Менеджер конфигурации
        std::shared_ptr<farm::config::ConfigManager> configManager;
        
        // MQTT клиент
        AsyncMqttClient mqttClient;
        
        // Информация о сервере MQTT
        String serverDomain;
        uint16_t serverPort = 0;
        
        // Время последней проверки соединения
        unsigned long lastCheckTime = 0;
        
        // Флаги состояния
        bool isConnecting = false;
        bool isConnected = false;
        bool isInitialized = false;
        
        // Счетчик попыток переподключения
        uint8_t reconnectAttempts = 0;
        
        // Время последней попытки переподключения
        unsigned long lastReconnectTime = 0;
        
        // Функции обратного вызова для MQTT
        void onMqttConnect(bool sessionPresent);
        void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
        void onMqttSubscribe(uint16_t packetId, uint8_t qos);
        void onMqttUnsubscribe(uint16_t packetId);
        void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, 
                          size_t len, size_t index, size_t total);
        void onMqttPublish(uint16_t packetId);
        
        // Настройка MQTT клиента
        bool setupMQTTClient();
        
        // Обработка полученной команды
        void handleCommand(const farm::config::CommandCode& command);

        // Имя устройства, буффер для setClientId (необходим для решения проблемы с const char* и c_str()!)
        char deviceIdBuffer[farm::config::mqtt::DEVICE_ID_MAX_LENGTH];
    public:
        // Получение экземпляра синглтона как shared_ptr
        static std::shared_ptr<MQTTManager> getInstance(std::shared_ptr<farm::log::ILogger> logger = nullptr);
        
        // Запрещаем копирование и присваивание
        MQTTManager(const MQTTManager&) = delete;
        MQTTManager& operator=(const MQTTManager&) = delete;
        
        // Деструктор
        ~MQTTManager();

        // Инициализация MQTT и попытка подключения
        bool initialize();
        
        // Методы для установки настроек MQTT
        bool setMqttHost(const String& host);
        bool setMqttPort(uint16_t port);
        bool setMqttDeviceId(const String& deviceId);

        // Получение текущих настроек MQTT
        String getMqttHost() const;
        uint16_t getMqttPort() const;
        String getMqttDeviceId() const;
        
        // Метод для одновременной установки всех параметров MQTT
        bool applyMqttSettings(const String& host, uint16_t port, const String& deviceId = "");
        
        // Поддержание MQTT соединения - вызывать в цикле loop()
        void maintainConnection();
        
        // Отправка данных в соответствующий топик
        bool publishData(uint8_t qos = farm::config::mqtt::QOS_1, bool retain = true);

        // Подписка на конкретный топик
        uint16_t subscribeToTopic(const String& topic, uint8_t qos = farm::config::mqtt::QOS_1);
        // Подписка на все стандартные топики
        bool subscribeToAllTopics(uint8_t qos = farm::config::mqtt::QOS_1);
        
        // Отписка от конкретного топика
        uint16_t unsubscribeFromTopic(const String& topic);
        // Отписка от всех подписанных топиков
        bool unsubscribeFromAllTopics();
        
        // Проверка состояния подключения
        bool isClientConnected() const;
        
        // Проверка, настроен ли MQTT
        bool isMqttConfigured() const;
        
        // Получить топик MQTT в зависимости от типа
        String getMqttTopic(farm::config::ConfigType type) const;
    };
}
