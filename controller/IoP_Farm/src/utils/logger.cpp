#include "utils/logger.h"
#include "network/mqtt_manager.h"
#include <stdarg.h>

namespace farm::log
{
    using namespace farm::config;
    using namespace farm::log;

    // Реализация SerialTransport
    void SerialTransport::write(Level level, const String& message)
    {
        Serial.println(message);
    }

    // Реализация StandardFormatter
    String StandardFormatter::format(Level level, const char* message) const
    {
        return String(getLevelPrefix(level)) + message;
    }

    const char* StandardFormatter::getLevelPrefix(Level level)
    {
        switch (level)
        {
            case Level::Error:   return constants::ERROR_PREFIX;
            case Level::Warning: return constants::WARN_PREFIX;
            case Level::Info:    return constants::INFO_PREFIX;
            case Level::Debug:   return constants::DEBUG_PREFIX;
            case Level::Test:    return constants::TEST_PREFIX;
            default:             return "";
        }
    }

    // Реализация ColorFormatter
    String ColorFormatter::format(Level level, const char* message) const
    {
        // Формируем сообщение с цветом
        return String(getLevelColor(level)) + String(getLevelPrefix(level)) + 
               message + String(constants::COLOR_RESET);
    }
    
    const char* ColorFormatter::getLevelColor(Level level)
    {
        switch (level)
        {
            case Level::Error:   return constants::COLOR_RED;
            case Level::Warning: return constants::COLOR_YELLOW;
            case Level::Info:    return constants::COLOR_WHITE;
            case Level::Debug:   return constants::COLOR_BLUE;
            case Level::Test:    return constants::COLOR_MAGENTA;
            default:             return constants::COLOR_WHITE;
        }
    }

    // Реализация MQTTLogTransport
    MQTTLogTransport::MQTTLogTransport() : lastSendTime(0), logBuffer()
    {
        // Конструктор
    }
    
    void MQTTLogTransport::write(Level level, const String& message)
    {
        // Проверяем, что уровень сообщения достаточен для отправки
        if (static_cast<int>(level) <= static_cast<int>(constants::MQTT_LOG_MIN_LEVEL))
        {
            // Проверяем размер буфера и удаляем старые сообщения при необходимости
            if (logBuffer.size() >= constants::MAX_BUFFER_SIZE)
            {
                logBuffer.erase(logBuffer.begin());
            }
            
            // Добавляем сообщение в буфер
            logBuffer.push_back(message);
                
            // Отправляем логи, если прошло достаточно времени и MQTT подключен
            processLogs();
        }
    }
    
    void MQTTLogTransport::flushLogs()
    {
        // Получаем экземпляр MQTTManager через синглтон
        auto mqttManager = farm::net::MQTTManager::getInstance();
        
        // Если буфер пуст или MQTT не настроен или не подключен, выходим
        if (logBuffer.empty() || !mqttManager->isMqttInitialized() || !mqttManager->isClientConnected()) 
        {
            return;
        }
        
        // Объединяем логи в одно сообщение с разделителями
        String combinedLog;
        for (const auto& log : logBuffer) 
        {
            combinedLog += log + "\n";
        }
        
        // Получаем топик для логов
        String logTopic = "/" + String(mqtt::DEFAULT_DEVICE_ID) + String(mqtt::LOG_SUFFIX);
        
        // Отправляем через MQTT с QoS = 1 и retain = true
        mqttManager->publishToTopicLoggerVersion(logTopic, combinedLog);
        
        // Очищаем буфер и обновляем время
        logBuffer.clear();
        lastSendTime = millis();
    }
    
    void MQTTLogTransport::processLogs()
    {
        // Получаем экземпляр MQTTManager через синглтон
        auto mqttManager = farm::net::MQTTManager::getInstance();
        
        // Проверяем, инициализирован ли MQTT
        if (!mqttManager->isMqttInitialized())
        {
            return;
        }
        
        // Проверяем, не пора ли отправить логи
        unsigned long currentTime = millis();
        if (!logBuffer.empty() && 
            (currentTime - lastSendTime >= constants::MQTT_LOG_SEND_INTERVAL) && 
            mqttManager->isClientConnected()) 
        {
            flushLogs();
        }
    }

    // Реализация Logger
    Logger::Logger(std::shared_ptr<IMessageFormatter> formatter)
        : level(Level::Info), formatter(std::move(formatter))
    {
        // Конструктор
    }

    void Logger::addTransport(std::shared_ptr<ILogTransport> transport)
    {
        transports.push_back(std::move(transport));
    }

    void Logger::log(Level level, const char* format, ...) const
    {
        if (!shouldLog(level)) return;

        char buffer[constants::LOG_BUFFER_SIZE];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        String formattedMessage = formatter->format(level, buffer);

        for (const auto& transport : transports)
        {
            transport->write(level, formattedMessage);
        }
    }

    void Logger::setLevel(Level level)
    {
        this->level = level;
    }

    Level Logger::getLevel() const
    {
        return level;
    }

    bool Logger::shouldLog(Level level) const
    {
        return static_cast<int>(level) <= static_cast<int>(this->level);
    }
} 