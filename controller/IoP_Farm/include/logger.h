#pragma once
#include <Arduino.h>

namespace farm::log
{
    // Уровни логирования по убыванию важности
    enum class Level
    {
        None,
        Error,
        Warning,
        Info,
        Debug
    };

    // Интерфейс логгера
    class ILogger
    {
    public:
        virtual void setLevel(Level level) = 0;
        virtual void error(const char* msg) const = 0;
        virtual void warning(const char* msg) const = 0;
        virtual void info(const char* msg) const = 0;
        virtual void debug(const char* msg) const = 0;
        virtual ~ILogger() = default;
    };

    // Реализация логгера через Serial
    class SerialLogger : public ILogger
    {
    public:
        void setLevel(Level level) override;
        void error(const char* msg) const override;
        void warning(const char* msg) const override;
        void info(const char* msg) const override;
        void debug(const char* msg) const override;

    private:
        static Level currentLevel;
    };
}