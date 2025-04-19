#include "../../include/utils/logger_factory.h"

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
} 