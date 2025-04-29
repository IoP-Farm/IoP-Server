#include "sensors/ISensor.h"

namespace farm::sensors
{
    // Используем пространство имен для логгера
    using namespace farm::log;
    using namespace farm::config::sensors;
    using namespace farm::config;
    
    // Конструктор
    ISensor::ISensor()
        : lastMeasurement(calibration::NO_DATA),
          shouldBeRead(true),
          shouldBeSaved(true),
          initialized(false),
          configManager(ConfigManager::getInstance()) {}
    
    // Установка имени датчика
    void ISensor::setSensorName(const String& name)
    {
        sensorName = name;
    }
    
    // Установка типа измеряемой величины
    void ISensor::setMeasurementType(const String& type)
    {
        measurementType = type;
    }
    
    // Установка единицы измерения
    void ISensor::setUnit(const String& unitValue)
    {
        unit = unitValue;
    }
    
    // Записать значение в память
    bool ISensor::saveMeasurement()
    {
        if (lastMeasurement == calibration::NO_DATA)
        {
            logger->log(Level::Error, "[Sensor] %s: нет данных для сохранения, сохранение для дальнейшего анализа", sensorName.c_str());
            configManager->setValue(ConfigType::Data, measurementType.c_str(), lastMeasurement);
            return false;
        }
        else if (lastMeasurement == calibration::SENSOR_ERROR_VALUE)
        {
            logger->log(Level::Error, "[Sensor] %s: сохранение ошибочного значения для дальнейшего анализа", sensorName.c_str()); 
            configManager->setValue(ConfigType::Data, measurementType.c_str(), lastMeasurement);
            return false;
        }
        else
        {
            // Сохраняем значение в память по ключу из measurementType
            configManager->setValue(ConfigType::Data, measurementType.c_str(), lastMeasurement);
        }

        return true;
    }
    
    // Получить последнее измеренное значение
    float ISensor::getLastMeasurement() const
    {
        if (lastMeasurement == calibration::NO_DATA)
        {
            logger->log(Level::Error, "[Sensor] %s: нет данных для чтения", sensorName.c_str());
            return calibration::SENSOR_ERROR_VALUE;
        }
        else if (lastMeasurement == calibration::SENSOR_ERROR_VALUE)
        {
            logger->log(Level::Error, "[Sensor] %s: ошибка чтения данных", sensorName.c_str());
            return calibration::SENSOR_ERROR_VALUE;
        }
        return lastMeasurement;
    }
    
    // Получить название датчика
    String ISensor::getSensorName() const
    {
        return sensorName;
    }
    
    // Получить тип измеряемой величины
    String ISensor::getMeasurementType() const
    {
        return measurementType;
    }
    
    // Получить единицу измерения
    String ISensor::getUnit() const
    {
        return unit;
    }
} 