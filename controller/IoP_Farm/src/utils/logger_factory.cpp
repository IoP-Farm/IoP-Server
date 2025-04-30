#include "utils/logger_factory.h"

namespace farm::log
{
    // Создание стандартного логгера с выводом в Serial
    std::shared_ptr<ILogger> LoggerFactory::createSerialLogger(Level level)
    {
        // Создаем форматтер
        auto formatter = std::make_shared<StandardFormatter>();
        
        // Создаем логгер
        auto logger = std::make_shared<Logger>(formatter);
        
        // Добавляем транспорт
        auto transport = std::make_shared<SerialTransport>();
        logger->addTransport(transport);
        
        // Устанавливаем уровень логирования
        logger->setLevel(level);
        
        return logger;
    }
    
    // Создание цветного логгера с выводом в Serial
    std::shared_ptr<ILogger> LoggerFactory::createColorSerialLogger(Level level)
    {
        // Создаем цветной форматтер
        auto formatter = std::make_shared<ColorFormatter>();
        
        // Создаем логгер
        auto logger = std::make_shared<Logger>(formatter);
        
        // Добавляем транспорт
        auto transport = std::make_shared<SerialTransport>();
        logger->addTransport(transport);
        
        // Устанавливаем уровень логирования
        logger->setLevel(level);
        
        return logger;
    }

    // Создать логгер с отправкой в MQTT
    std::shared_ptr<ILogger> LoggerFactory::createMQTTLogger(Level level)
    {
        // Создаем цветной форматтер
        auto formatter = std::make_shared<StandardFormatter>();
        
        // Создаем логгер
        auto logger = std::make_shared<Logger>(formatter);
        
        // Добавляем транспорт
        auto transport = std::make_shared<MQTTLogTransport>();
        logger->addTransport(transport);
        
        // Устанавливаем уровень логирования
        logger->setLevel(level);
        
        return logger;
    }

    // Создать цветной логгер с отправкой в MQTT
    std::shared_ptr<ILogger> LoggerFactory::createColorMQTTLogger(Level level)
    {
        // Создаем цветной форматтер
        auto formatter = std::make_shared<ColorFormatter>();
        
        // Создаем логгер
        auto logger = std::make_shared<Logger>(formatter);

        // Добавляем транспорт Serial
        auto serialTransport = std::make_shared<SerialTransport>();
        logger->addTransport(serialTransport);
        
        // Добавляем транспорт MQTT
        auto mqttTransport = std::make_shared<MQTTLogTransport>();
        logger->addTransport(mqttTransport);
        
        // Устанавливаем уровень логирования
        logger->setLevel(level);
        
        return logger;
    }

    // Создать логгер с отправкой в MQTT и Serial
    std::shared_ptr<ILogger> LoggerFactory::createSerialMQTTLogger(Level level)
    {
        // Создаем форматтер
        auto formatter = std::make_shared<StandardFormatter>();
        
        // Создаем логгер
        auto logger = std::make_shared<Logger>(formatter);

        // Добавляем транспорт Serial
        auto serialTransport = std::make_shared<SerialTransport>();
        logger->addTransport(serialTransport);
        
        // Добавляем транспорт MQTT
        auto mqttTransport = std::make_shared<MQTTLogTransport>();
        logger->addTransport(mqttTransport);

        // Устанавливаем уровень логирования
        logger->setLevel(level);

        return logger;
    }

    // Создать цветной логгер с отправкой в MQTT и Serial
    std::shared_ptr<ILogger> LoggerFactory::createColorSerialMQTTLogger(Level level)
    {
        // Создаем цветной форматтер
        auto formatter = std::make_shared<ColorFormatter>();
        
        // Создаем логгер
        auto logger = std::make_shared<Logger>(formatter);

        // Добавляем транспорт Serial
        auto serialTransport = std::make_shared<SerialTransport>();
        logger->addTransport(serialTransport);
        
        // Добавляем транспорт MQTT
        auto mqttTransport = std::make_shared<MQTTLogTransport>();
        logger->addTransport(mqttTransport);

        // Устанавливаем уровень логирования
        logger->setLevel(level);
        return logger;
    }
} 