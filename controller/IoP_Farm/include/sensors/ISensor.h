#pragma once

#include <Arduino.h>
#include <memory>
#include "utils/logger_factory.h"
#include "config/config_manager.h"
#include "config/constants.h"

namespace farm::sensors
{
    using namespace farm::log;
    using namespace farm::config;
    using namespace farm::config::sensors;

    // Абстрактный класс датчика
    class ISensor
    {
    protected:
        // Имя датчика
        String sensorName;
        
        // Тип измеряемой величины
        String measurementType;

        // Единица измерения
        String unit;
        
        // Последнее измеренное значение
        float lastMeasurement;
        
        // Логгер
        std::shared_ptr<log::ILogger> logger;
        
        // Менеджер конфигурации
        std::shared_ptr<config::ConfigManager> configManager;
        
        // Защищенные методы для установки параметров в наследниках
        void setSensorName(const String& name);
        void setMeasurementType(const String& type);
        void setUnit(const String& unit);
        
    public:
        // Флаги для определения поведения датчика
        bool shouldBeRead;
        bool shouldBeSaved;
        
        // Флаг инициализации датчика
        bool initialized;
    public:
        // Конструктор
        ISensor();

        // Виртуальный деструктор
        virtual ~ISensor() = default;
        
        // Считать значение с датчика и записать в lastMeasurement, вернуть его
        virtual float read() = 0;
        
        // Записать значение в память
        virtual bool saveMeasurement();
        
        // Получить последнее измеренное значение
        float getLastMeasurement() const;
        
        // Получить название датчика
        String getSensorName() const;
        
        // Получить тип измеряемой величины
        String getMeasurementType() const;
        
        // Получить единицу измерения
        String getUnit() const;
        
        // Инициализация датчика
        virtual bool initialize() = 0;
    };
} 

