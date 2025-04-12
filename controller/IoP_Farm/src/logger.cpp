#include "logger.h"

namespace farm::log
{
    Level SerialLogger::currentLevel = Level::Info; // Изначально уровень логирования - Info

    void SerialLogger::setLevel(Level level)
    {
        currentLevel = level;
    }

    void SerialLogger::error(const char* msg) const
    {
        if (currentLevel >= Level::Error)
        {
            Serial.print("[ERROR] ");
            Serial.println(msg);
        }
    }

    void SerialLogger::warning(const char* msg) const
    {
        if (currentLevel >= Level::Warning)
        {
            Serial.print("[WARN] ");
            Serial.println(msg);
        }
    }

    void SerialLogger::info(const char* msg) const
    {
        if (currentLevel >= Level::Info)
        {
            Serial.print("[INFO] ");
            Serial.println(msg);
        }
    }

    void SerialLogger::debug(const char* msg) const
    {
        if (currentLevel >= Level::Debug)
        {
            Serial.print("[DEBUG] ");
            Serial.println(msg);
        }
    }
}
