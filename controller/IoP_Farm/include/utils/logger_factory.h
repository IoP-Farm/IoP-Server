#pragma once
#include "logger.h"
#include <memory>

namespace farm::log
{
    // Фабрика для создания предварительно настроенных логгеров
    class LoggerFactory
    {
    public:
        // Создать стандартный логгер с Serial-выводом
        static std::shared_ptr<ILogger> createSerialLogger(Level level = Level::Info);
        
        // Создать цветной логгер с Serial-выводом
        static std::shared_ptr<ILogger> createColorSerialLogger(Level level = Level::Info);
        
        // Создать логгер с отправкой в MQTT
        static std::shared_ptr<ILogger> createMQTTLogger(Level level = Level::Info);

        // Создать цветной логгер с отправкой в MQTT
        static std::shared_ptr<ILogger> createColorMQTTLogger(Level level = Level::Info);

        // Создать логгер с отправкой в MQTT и Serial
        static std::shared_ptr<ILogger> createSerialMQTTLogger(Level level = Level::Info);
        
        // Создать цветной логгер с отправкой в MQTT и Serial
        static std::shared_ptr<ILogger> createColorSerialMQTTLogger(Level level = Level::Info);
    };
} 