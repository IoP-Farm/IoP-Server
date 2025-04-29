// Датчик влажности почвы

#include "sensors/FC28.h"
#include "utils/my_map.h"

namespace farm::sensors
{
    // Конструктор
    FC28::FC28(std::shared_ptr<log::ILogger> logger, uint8_t pin)
        : pin(pin),
          dryValue(calibration::FC28_DRY_VALUE),
          wetValue(calibration::FC28_WET_VALUE)
    {
        // Сохраняем логгер
        this->logger = logger;
        
        // Устанавливаем параметры сенсора
        setSensorName(names::FC28);
        setMeasurementType(json_keys::SOIL_MOISTURE);
        setUnit(units::PERCENT);
        
        // Разрешаем считывание и сохранение данных
        shouldBeRead = true;
        shouldBeSaved = true;
        
        // Инициализируем значение как "нет данных"
        lastMeasurement = calibration::NO_DATA;
    }
    
    // Инициализация датчика
    bool FC28::initialize()
    {
        // Проверяем, что пин в допустимом диапазоне
        if (pin == calibration::UNINITIALIZED_PIN)
        {
            logger->log(Level::Error, 
                     "[FC28] Не задан пин для датчика влажности почвы");
            return false;
        }
        
        // Настраиваем пин как вход
        pinMode(pin, INPUT);
        
        initialized = true;
        return true;
    }
    
    // Считать влажность почвы в процентах (0-100%)
    float FC28::read()
    {
        if (!initialized)
        {
            logger->log(Level::Error, 
                     "[FC28] Датчик не инициализирован");
            lastMeasurement = calibration::SENSOR_ERROR_VALUE;
            return calibration::SENSOR_ERROR_VALUE;
        }
        
        // Считываем сырое значение АЦП
        int rawValue = getRawValue();
        
        // Проверяем корректность калибровочных значений
        if (dryValue <= wetValue)
        {
            logger->log(Level::Error, 
                      "[FC28] Некорректные калибровочные значения: dryValue=%d, wetValue=%d", 
                      dryValue, wetValue);
            lastMeasurement = calibration::SENSOR_ERROR_VALUE;
            return calibration::SENSOR_ERROR_VALUE;
        }
        
        // Преобразуем в проценты влажности (0% - сухо, 100% - мокро)
        // Используем ограничение на случай, если значения выходят за пределы калибровки
        int constrained = constrain(rawValue, wetValue, dryValue);
        float moisturePercent = utils::MyMap<int, float>(constrained, dryValue, wetValue, 0.0f, 100.0f);

        // Сохраняем результат
        lastMeasurement = moisturePercent;
        
        return moisturePercent;
    }
    
    // Получить текущее сырое значение АЦП
    int FC28::getRawValue() const
    {
        return analogRead(pin);
    }
    
    // Установить калибровочные значения
    void FC28::setCalibration(int dryValue, int wetValue)
    {
        // Проверяем корректность значений (сухое значение должно быть больше влажного)
        if (dryValue <= wetValue)
        {
            logger->log(Level::Error, 
                      "[FC28] Некорректные калибровочные значения: dryValue=%d должно быть > wetValue=%d", 
                      dryValue, wetValue);
            return;
        }
        
        this->dryValue = dryValue;
        this->wetValue = wetValue;
        
        logger->log(Level::Info, 
                  "[FC28] Установлены калибровочные значения: dryValue=%d, wetValue=%d", 
                  dryValue, wetValue);
    }
} 