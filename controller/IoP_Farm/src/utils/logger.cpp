#include "../../include/utils/logger.h"
#include <stdarg.h>

namespace farm::log
{
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

    const char* StandardFormatter::getLevelPrefix(Level level) const
    {
        switch (level)
        {
            case Level::Error:   return constants::ERROR_PREFIX;
            case Level::Warning: return constants::WARN_PREFIX;
            case Level::Info:    return constants::INFO_PREFIX;
            case Level::Debug:   return constants::DEBUG_PREFIX;
            case Level::Test:    return constants::TEST_PREFIX;
            default:            return "";
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