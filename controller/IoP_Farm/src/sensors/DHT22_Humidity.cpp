#include "sensors/DHT22_Humidity.h"
#include "sensors/DHT22Common.h"

namespace farm::sensors
{
    // Конструктор
    DHT22_Humidity::DHT22_Humidity(std::shared_ptr<log::ILogger> logger, uint8_t pin)
        : pin(pin),
          lastReadTime(0) // Инициализация времени последнего чтения
    {
        // Сохраняем логгер
        this->logger = logger;
        
        // Устанавливаем параметры датчика
        setSensorName(names::DHT22_HUMIDITY);
        setMeasurementType(json_keys::HUMIDITY);
        setUnit(units::PERCENT);
        
        // Разрешаем считывание и сохранение данных
        shouldBeRead = true;
        shouldBeSaved = true;
        
        // Инициализируем значение как "нет данных"
        lastMeasurement = calibration::NO_DATA;
        
        // Пока не создаем объект DHT, это делаем в initialize()
        dht = nullptr;
    }
    
    // Деструктор - не удаляем DHT, так как он может использоваться другими классами
    DHT22_Humidity::~DHT22_Humidity()
    {
        // Память будет освобождена автоматически благодаря std::shared_ptr
    }
    
    // Инициализация датчика
    bool DHT22_Humidity::initialize()
    {
        // Проверяем, что пин в допустимом диапазоне
        if (pin == calibration::UNINITIALIZED_PIN)
        {
            logger->log(Level::Error, 
                     "[DHT22_Humidity] Не задан пин для датчика");
            return false;
        }
        
        // Получаем объект DHT из общего хранилища
        dht = DHT22Common::getInstance(pin, logger);
        
        if (!dht)
        {
            logger->log(Level::Error, 
                     "[DHT22_Humidity] Ошибка при получении объекта DHT");
            return false;
        }
        
        initialized = true;
        return true;
    }
    
    // Считать влажность в процентах (0-100%)
    float DHT22_Humidity::read()
    {
        if (!initialized || !dht)
        {
            logger->log(Level::Error, 
                     "[DHT22_Humidity] Датчик не инициализирован");
            lastMeasurement = calibration::SENSOR_ERROR_VALUE;
            return calibration::SENSOR_ERROR_VALUE;
        }
        
        // Получаем текущее время
        unsigned long currentTime = millis();
        
        // Проверяем, прошло ли достаточно времени с момента последнего чтения
        if (lastReadTime > 0 && (currentTime - lastReadTime < MIN_READ_INTERVAL))
        {
            // Если прошло меньше 2 секунд, возвращаем последнее измеренное значение
            // без обращения к датчику
            return lastMeasurement;
        }
        
        // Считываем влажность
        float humidity = dht->readHumidity();
        
        // Обновляем время последнего чтения
        lastReadTime = currentTime;
        
        // Проверяем корректность считанного значения
        if (isnan(humidity)) 
        {
            logger->log(Level::Error, 
                      "[DHT22_Humidity] Не удалось считать данные с датчика");
            lastMeasurement = calibration::SENSOR_ERROR_VALUE;
            return calibration::SENSOR_ERROR_VALUE;
        }
        
        // Сохраняем значение
        lastMeasurement = humidity;
        
        return humidity;
    }
} 