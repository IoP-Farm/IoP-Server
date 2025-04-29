#pragma once

#include <map>
#include <memory>
#include "sensors/ISensor.h"
#include "sensors/DHT22_Temperature.h"
#include "sensors/DHT22_Humidity.h"
#include "sensors/DS18B20.h"
#include "sensors/FC28.h"
#include "sensors/HCSR04.h"
#include "sensors/KY018.h"
#include "sensors/YFS401.h"
#include "config/config_manager.h"
#include "utils/logger_factory.h"
#include "network/mqtt_manager.h"

namespace farm::sensors
{
    using namespace farm::log;
    using namespace farm::config;
    using namespace farm::net;
    
    class SensorsManager
    {
    private:
        // Приватный конструктор (паттерн Синглтон)
        explicit SensorsManager(std::shared_ptr<log::ILogger> logger = nullptr);
        
        // Статический экземпляр
        static std::shared_ptr<SensorsManager> instance;
        
        // Менеджер конфигурации
        std::shared_ptr<config::ConfigManager> configManager;
        
        // Логгер
        std::shared_ptr<log::ILogger> logger;

        // Ссылка на MQTT менеджер
        std::shared_ptr<net::MQTTManager> mqttManager;

        // Последний момент времени чтения датчиков
        unsigned long lastReadTime;
        
        // Интервал чтения датчиков (мс)
        // Инициализируется константой, но может быть изменено (закладка на будущее)
        unsigned long readInterval;
        
        // Обобщенный шаблонный метод для создания и инициализации датчика
        template<typename SensorType, typename... Args>
        bool createAndInitSensor(Args&&... args);
        
    public:
        // Публичный map датчиков для удобства использования [имя датчика, указатель на датчик]
        std::map<String, std::shared_ptr<ISensor>> sensors;
        
        // Получение экземпляра синглтона
        static std::shared_ptr<SensorsManager> getInstance(std::shared_ptr<log::ILogger> logger = nullptr);
        
        // Запрещаем копирование и присваивание
        SensorsManager(const SensorsManager&) = delete;
        SensorsManager& operator=(const SensorsManager&) = delete;
        
        // Деструктор
        ~SensorsManager();
        
        // Инициализация всех датчиков
        bool initialize();
        
        // Считать значения со всех датчиков
        bool readAllSensors();
        
        // Сохранить все измеренные значения
        bool saveAllMeasurements();
        
        // Метод для периодического выполнения в loop
        bool loop();
        
        // Получить последнее измеренное значение с датчика по имени
        float getLastMeasurement(const String& sensorName);

        // Считать значение с датчика по имени
        float sensorRead(const String& sensorName);

        // Сохранить значение с датчика по имени
        bool sensorSave(const String& sensorName);

        // считать и сохранить значение с датчика по имени
        float sensorReadAndSave(const String& sensorName);

        // получить тип измеряемой величины с датчика по имени
        String getSensorType(const String& sensorName);

        // получить единицу измерения с датчика по имени
        String getSensorUnit(const String& sensorName);

        // Установка интервала считывания
        void setReadInterval(unsigned long interval);
        
        // Добавить датчик
        bool addSensor(const String& sensorName, std::shared_ptr<ISensor> sensor);
        
        // Удалить датчик
        bool removeSensor(const String& sensorName);
        
        // Получить датчик по имени
        std::shared_ptr<ISensor> getSensor(const String& sensorName);
        
        // Проверить, есть ли такой датчик
        bool hasSensor(const String& sensorName) const;
        
        // Получить количество датчиков
        size_t getSensorCount() const;
        
        // Очистить все датчики
        void clearSensors();
    };
    
    // Реализация шаблонного метода (должна быть в заголовочном файле)
    template<typename SensorType, typename... Args>
    bool SensorsManager::createAndInitSensor(Args&&... args)
    {
        auto sensor = std::make_shared<SensorType>(logger, std::forward<Args>(args)...);

        if (sensor->initialize()) 
        {
            addSensor(sensor->getSensorName(), sensor);
            return true;
        } 
        else 
        {
            logger->log(farm::log::Level::Warning, "[Sensors] Не удалось инициализировать датчик %s (%s)", 
                      sensor->getSensorName(), sensor->getMeasurementType().c_str());
            return false;
        }
    }
} 