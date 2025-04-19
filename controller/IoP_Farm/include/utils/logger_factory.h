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
    };
} 