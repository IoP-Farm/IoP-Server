#pragma once
#include <Arduino.h>
#include <string>
#include <vector>
#include <memory>

namespace farm::log
{
    // Константы для логгера
    namespace constants
    {
        constexpr size_t LOG_BUFFER_SIZE   = 256;  // Размер буфера для форматирования сообщений
        constexpr const char* ERROR_PREFIX = "[ERROR] ";
        constexpr const char* WARN_PREFIX  = "[WARN]  ";
        constexpr const char* INFO_PREFIX  = "[INFO]  ";
        constexpr const char* DEBUG_PREFIX = "[DEBUG] ";
        constexpr const char* TEST_PREFIX  = "[TEST]  ";
    }

    // Уровни логирования
    enum class Level
    {
        None,
        Error,
        Warning,
        Info,
        Debug,
        Test
    };

    // Транспорт для вывода сообщений
    class ILogTransport
    {
    public:
        virtual ~ILogTransport() = default;
        virtual void write(Level level, const String& message) = 0;
    };
    
    // Транспорт для Serial
    class SerialTransport : public ILogTransport
    {
    public:
        void write(Level level, const String& message) override;
    };

    // Форматтер для сообщений
    class IMessageFormatter
    {
    public: 
        virtual ~IMessageFormatter() = default;
        virtual String format(Level level, const char* message) const = 0;
    };
    
    // Стандартный форматтер с префиксами
    class StandardFormatter : public IMessageFormatter
    {
    public:
        String format(Level level, const char* message) const override;
        
    private:
        const char* getLevelPrefix(Level level) const;
    };

    // Интерфейс логгера
    class ILogger
    {
    public:
        virtual void log(Level level, const char* format, ...) const = 0;
        virtual void setLevel(Level level) = 0;
        virtual Level getLevel() const = 0;
        virtual ~ILogger() = default;
    };

    // Реализация логгера
    class Logger : public ILogger
    {
    private:
        Level level;
        std::vector<std::shared_ptr<ILogTransport>> transports;
        std::shared_ptr<IMessageFormatter> formatter;
        
    public:
        Logger(std::shared_ptr<IMessageFormatter> formatter);
        
        void addTransport(std::shared_ptr<ILogTransport> transport);
        void log(Level level, const char* format, ...) const override;
        void setLevel(Level level) override;
        Level getLevel() const override;
        
    private:
        bool shouldLog(Level level) const;
    };
} 