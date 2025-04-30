#include "sensors/sensors_manager.h"
#include "sensors/DHT22Common.h"
#include "sensors/DS18B20Common.h"

namespace farm::sensors
{
    // Используем пространства имен для удобства
    using namespace farm::log;
    using namespace farm::config;
    using namespace farm::config::sensors;
    
    // Инициализация статического экземпляра
    std::shared_ptr<SensorsManager> SensorsManager::instance = nullptr;
    
    // Конструктор
    SensorsManager::SensorsManager(std::shared_ptr<log::ILogger> logger)
        : lastReadTime(0),
          readInterval(timing::DEFAULT_READ_INTERVAL) // Используем константу из файла constants.h
    {
        // Если логгер не передан, создаем новый с помощью фабрики
        if (logger == nullptr) 
        {
            this->logger = LoggerFactory::createSerialLogger(Level::Info);
        } 
        else 
        {
            this->logger = logger;
        }
        
        // Получаем экземпляр ConfigManager
        configManager = ConfigManager::getInstance(this->logger);
        
        // Получаем экземпляр MQTTManager для публикации данных
        mqttManager = farm::net::MQTTManager::getInstance(this->logger);
    }
    
    // Получение экземпляра синглтона
    std::shared_ptr<SensorsManager> SensorsManager::getInstance(std::shared_ptr<ILogger> logger)
    {
        if (instance == nullptr) 
        {
            // Используем явное создание вместо make_shared, т.к. конструктор приватный
            instance = std::shared_ptr<SensorsManager>(new SensorsManager(logger));
        }
        return instance;
    }
    
    // Деструктор
    SensorsManager::~SensorsManager()
    {
        logger->log(Level::Debug, "[Sensors] Освобождение ресурсов SensorsManager");
        
        // Очищаем датчики
        clearSensors();
        
        // Освобождаем статические ресурсы
        DHT22Common::releaseAll();
        DS18B20Common::releaseAll();
        
        logger->log(Level::Debug, "[Sensors] Ресурсы освобождены");
    }

    
    // Инициализация всех датчиков
    bool SensorsManager::initialize()
    {
        logger->log(Level::Farm, "[Sensors] Инициализация датчиков");
        
        // Очищаем существующие датчики
        clearSensors();
        
        bool allInitialized = true;
        
        // Создаем и инициализируем датчики если пины настроены
        
        int totalChecked = 0;

        // Датчик DHT22 (температура)
        if (pins::DHT22_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<DHT22_Temperature>(pins::DHT22_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        // Датчик DHT22 (влажность)
        if (pins::DHT22_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<DHT22_Humidity>(pins::DHT22_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        // Датчик DS18B20 (температура воды)
        if (pins::DS18B20_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<DS18B20>(pins::DS18B20_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        // Датчик HC-SR04 (уровень воды)
        if (pins::HC_SR04_TRIG_PIN != calibration::UNINITIALIZED_PIN && pins::HC_SR04_ECHO_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<HCSR04>(pins::HC_SR04_TRIG_PIN, pins::HC_SR04_ECHO_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        // Датчик FC-28 (влажность почвы)
        if (pins::FC28_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<FC28>(pins::FC28_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        // Датчик KY-018 (освещённость)
        if (pins::KY018_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<KY018>(pins::KY018_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        // Датчик YF-S401 (расходомер)
        if (pins::YFS401_PIN != calibration::UNINITIALIZED_PIN) 
        {
            if (!createAndInitSensor<YFS401>(pins::YFS401_PIN)) 
            {
                allInitialized = false;
            }
            totalChecked++;
        }
        
        logger->log(Level::Farm, 
                  "[Sensors] Количество инициализированных датчиков: %d/%d", 
                  sensors.size(), totalChecked);
        
        return allInitialized;
    }
    
    // Считать значения со всех датчиков (без сохранения)
    bool SensorsManager::readAllSensors()
    {
        bool allSuccess = true;
        int successCount = 0;
        int totalReadable = 0;
        
        for (auto& [name, sensor] : sensors) 
        {
            // Проверяем, должен ли этот датчик считывать данные
            if (!sensor->shouldBeRead) {
                continue;
            }
            
            totalReadable++;
            
            // Считываем показания
            if (sensor->read() != calibration::SENSOR_ERROR_VALUE) 
            {
                logger->log(Level::Info, 
                          "[Sensors] [%s] %s = %.2f %s", 
                          name.c_str(), 
                          sensor->getMeasurementType().c_str(), 
                          sensor->getLastMeasurement(), 
                          sensor->getUnit().c_str());
                successCount++;
            } 
            else 
            {
                logger->log(Level::Error, 
                          "[Sensors] Ошибка считывания данных с датчика %s (%s)", 
                          name.c_str(), 
                          sensor->getMeasurementType().c_str());
                allSuccess = false;
            }
        }
        
        logger->log(Level::Farm, 
                  "[Sensors] Успешно считано: %d/%d", 
                  successCount, totalReadable);
        
        return allSuccess;
    }
    
    // Сохранить все измеренные значения в оперативную память (Json)
    bool SensorsManager::saveAllMeasurements()
    {
        bool allSuccess = true;
        int successCount = 0;
        int totalSaveable = 0;
        
        for (auto& [name, sensor] : sensors) 
        {
            // Проверяем, должен ли этот датчик сохранять данные
            if (!sensor->shouldBeSaved) {
                continue;
            }
            
            totalSaveable++;

            if (sensor->saveMeasurement()) 
            {
                successCount++;
            } 
            else 
            {
                logger->log(Level::Error, 
                          "[Sensors] Ошибка сохранения данных с датчика %s (%s)", 
                          name.c_str(), 
                          sensor->getMeasurementType().c_str());
                allSuccess = false;
            }
        }
        
        logger->log(Level::Farm, 
                  "[Sensors] Успешно сохранено: %d/%d", 
                  successCount, totalSaveable);
        
        return allSuccess;
    }
    
    // Метод для периодического выполнения в loop
    bool SensorsManager::loop()
    {
        // Проверяем, прошло ли достаточно времени для нового считывания
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime >= readInterval) 
        {
            lastReadTime = currentTime;
            
            logger->log(Level::Farm, "[Sensors] Запуск цикла считывания данных");
            
            // Считываем данные со всех датчиков
            readAllSensors();
            
            // Сохраняем измеренные значения в оперативную память (Json)
            saveAllMeasurements();

            // Сохраняем конфигурацию в SPIFFS
            configManager->saveConfig(ConfigType::Data);
            
            // Если подключены к MQTT, публикуем данные
            if (mqttManager && mqttManager->isClientConnected()) 
            {
                bool published = mqttManager->publishData();
                
                if (!published) 
                {
                    logger->log(Level::Error, "[Sensors] Ошибка публикации данных в MQTT");
                }
                
                return true; // Продолжаем работу, даже если не удалось опубликовать данные
            }
            else
            {
                logger->log(Level::Warning, "[Sensors] Нет подключения к MQTT для отправки данных");
                return false;
            }
        }
        
        return true;
    }

    float SensorsManager::sensorRead(const String &sensorName)
    {
        return sensors[sensorName]->read();
    }

    bool SensorsManager::sensorSave(const String &sensorName)
    {
        return sensors[sensorName]->saveMeasurement();
    }

    float SensorsManager::sensorReadAndSave(const String &sensorName)
    {
        float value = sensorRead(sensorName);
        sensorSave(sensorName);
        return value;
    }

    String SensorsManager::getSensorType(const String &sensorName)
    {
        return sensors[sensorName]->getMeasurementType();
    }

    String SensorsManager::getSensorUnit(const String &sensorName)
    {
        return sensors[sensorName]->getUnit();
    }
    
    float SensorsManager::getLastMeasurement(const String &sensorName)
    {
        return sensors[sensorName]->getLastMeasurement();
    } 



    // Добавить датчик
    bool SensorsManager::addSensor(const String& sensorName, std::shared_ptr<ISensor> sensor)
    {
        // Проверяем, есть ли уже датчик с таким именем
        if (sensors.find(sensorName) != sensors.end()) 
        {
            logger->log(Level::Warning, 
                      "[Sensors] Датчик с именем %s (%s) уже существует", 
                      sensorName.c_str(), 
                      sensor->getMeasurementType().c_str());
            return false;
        }
        
        // Проверяем инициализацию и инициализируем при необходимости
        if (!sensor->initialized) 
        {
            if (!sensor->initialize()) 
            {
                logger->log(Level::Error, 
                          "[Sensors] Не удалось инициализировать датчик %s (%s)", 
                          sensorName.c_str(), 
                          sensor->getMeasurementType().c_str());
                return false;
            }
        }
        
        // Добавляем датчик в map
        sensors[sensorName] = sensor;
        logger->log(Level::Info, 
                  "[Sensors] Датчик %s (%s) добавлен в SensorsManager", 
                  sensorName.c_str(), 
                  sensor->getMeasurementType().c_str());
        
        return true;
    }

    // Установка интервала считывания
    void SensorsManager::setReadInterval(unsigned long interval)
    {
        readInterval = interval;
        logger->log(Level::Info, 
                  "[Sensors] Установлен интервал считывания датчиков: %lu мс", 
                  interval);
    }

    // Удалить датчик
    bool SensorsManager::removeSensor(const String& sensorName)
    {
        // Проверяем, существует ли датчик
        auto it = sensors.find(sensorName);
        if (it == sensors.end()) 
        {
            logger->log(Level::Warning, 
                      "[Sensors] Датчик с именем %s не найден", 
                      sensorName.c_str());
            return false;
        }
        
        // Удаляем датчик из map
        sensors.erase(it);
        logger->log(Level::Debug, 
                  "[Sensors] Датчик %s удален", 
                  sensorName.c_str());
        
        return true;
    }
    
    // Получить датчик по имени
    std::shared_ptr<ISensor> SensorsManager::getSensor(const String& sensorName)
    {
        auto it = sensors.find(sensorName);
        if (it != sensors.end()) 
        {
            return it->second;
        }
        
        logger->log(Level::Warning, 
                  "[Sensors] Датчик с именем %s не найден", 
                  sensorName.c_str());
        
        return nullptr;
    }
    
    // Проверить, есть ли такой датчик
    bool SensorsManager::hasSensor(const String& sensorName) const
    {
        return sensors.find(sensorName) != sensors.end();
    }
    
    // Получить количество датчиков
    size_t SensorsManager::getSensorCount() const
    {
        return sensors.size();
    }
    
    // Очистить все датчики
    void SensorsManager::clearSensors()
    {
        sensors.clear();
        logger->log(Level::Debug, "[Sensors] Все датчики удалены");
    }
} 