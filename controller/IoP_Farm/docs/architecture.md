# Архитектура IoP-Farm

## Общее описание

IoP-Farm представляет собой систему автоматизированного управления растениями в закрытом грунте, разработанную на базе микроконтроллера ESP32. Система предназначена для создания оптимальных условий выращивания растений путем автоматизации контроля и управления параметрами окружающей среды: температурой, влажностью воздуха, влажностью почвы, уровнем освещения и полива.

## Ключевые компоненты системы

Система спроектирована по модульному принципу и состоит из следующих основных компонентов:

### 1. Ядро системы
- `IoPCore` - центральный модуль, координирующий работу всех подсистем
- Система конфигурации
- Система логирования
- Планировщик задач для периодических операций

### 2. Сетевой уровень
- `INetworkManager` - базовый интерфейс для сетевых взаимодействий
- `WiFiManager` - модуль управления WiFi-соединением
- `MqttClient` - клиент для коммуникации по MQTT протоколу

### 3. Уровень периферийных устройств
- `ISensor` - базовый интерфейс для всех типов датчиков
- `IActuator` - базовый интерфейс для всех типов исполнительных устройств
- Конкретные реализации датчиков и исполнительных устройств

### 4. Уровень бизнес-логики
- `SystemStateManager` - менеджер состояния системы
- `EnvironmentController` - контроллер окружающей среды
- `IrrigationController` - контроллер системы полива
- `LightingController` - контроллер системы освещения
- `HeatingController` - контроллер системы обогрева

## Подробное описание компонентов

### Ядро системы

#### IoPCore
Основной класс, инициализирующий все компоненты системы и координирующий их взаимодействие. Отвечает за запуск системы, мониторинг её состояния и корректное завершение работы.

```cpp
class IoPCore {
public:
    IoPCore();
    ~IoPCore();
    
    bool initialize();
    void run();
    void shutdown();
    
private:
    // Остальные компоненты системы
    farm::log::ILogger& logger;
    farm::net::INetworkManager& networkManager;
    std::vector<farm::io::sensors::ISensor*> sensors;
    std::vector<farm::io::actuators::IActuator*> actuators;
    farm::config::ConfigManager configManager;
    farm::system::SystemStateManager stateManager;
    farm::tasks::TaskScheduler scheduler;
    
    void initializeSensors();
    void initializeActuators();
    void setupTasks();
};
```

#### Система логирования
Отвечает за логирование информации о работе системы, ошибках и событиях.

```cpp
namespace farm::log {
    enum class Level {
        None,
        Error,
        Warning,
        Info,
        Debug,
        Test
    };
    
    class ILogger {
    public:
        virtual void log(Level level, const char* format, ...) const = 0;
        virtual void setLevel(Level level) = 0;
        virtual Level getLevel() const = 0;
        virtual ~ILogger() = default;
    };
    
    class SerialLogger : public ILogger {
        // Реализация логгера через Serial
    };
}
```

#### Система конфигурации
Управляет настройками системы, загрузкой и сохранением конфигурации.

```cpp
namespace farm::config {
    class ConfigManager {
    public:
        ConfigManager();
        ~ConfigManager();
        
        bool loadConfig();
        bool saveConfig();
        
        template <typename T>
        T getValue(const std::string& key, const T& defaultValue = T());
        
        template <typename T>
        void setValue(const std::string& key, const T& value);
        
    private:
        std::map<std::string, std::variant<int, float, std::string, bool>> configValues;
    };
}
```

#### Планировщик задач
Обеспечивает выполнение периодических задач и операций по расписанию.

```cpp
namespace farm::tasks {
    using TaskCallback = std::function<void()>;
    
    struct Task {
        TaskCallback callback;
        unsigned long interval;
        unsigned long lastRun;
        bool isEnabled;
        std::string name;
    };
    
    class TaskScheduler {
    public:
        TaskScheduler();
        
        void addTask(const std::string& name, TaskCallback callback, unsigned long interval);
        void removeTask(const std::string& name);
        void enableTask(const std::string& name, bool enable);
        void update();
        
    private:
        std::vector<Task> tasks;
    };
}
```

### Сетевой уровень

#### INetworkManager
Базовый интерфейс для всех сетевых менеджеров, обеспечивающий единый интерфейс взаимодействия.

```cpp
namespace farm::net {
    enum class NetworkError {
        None,
        Timeout,
        InvalidCredentials,
        ConnectionFailed,
        Unknown
    };
    
    struct NetworkResult {
        bool success;
        NetworkError error;
        std::string errorMessage;
    };
    
    class INetworkManager {
    public:
        virtual ~INetworkManager() = default;
        
        virtual bool initialize() = 0;
        virtual NetworkResult connect() = 0;
        virtual bool disconnect() = 0;
        virtual bool isConnected() = 0;
    };
}
```

#### WiFiManager
Реализация сетевого менеджера для управления WiFi-соединением.

```cpp
namespace farm::net {
    struct WiFiNetworkInfo {
        String ssid;
        int rssi;
        int channel;
        wifi_auth_mode_t authMode;
        bool isConnected;
        IPAddress ipAddress;
    };
    
    class NetworkList {
        // Класс для хранения и управления списком сетей
    };
    
    class WiFiManager : public INetworkManager {
    public:
        explicit WiFiManager(const farm::log::ILogger& logger);
        ~WiFiManager() override;
        
        bool initialize() override;
        NetworkResult connect() override;
        bool disconnect() override;
        bool isConnected() override;
        
        NetworkList scanNetworks();
        void logNetworks(const NetworkList& networks, size_t maxNetworks = 0) const;
        void logNetworkInfo(const WiFiNetworkInfo& network) const;
        
    private:
        const farm::log::ILogger& logger;
        bool isConnecting;
    };
}
```

#### MqttClient
Реализация клиента для взаимодействия по протоколу MQTT.

```cpp
namespace farm::net {
    using MqttMessageCallback = std::function<void(const String&, const String&)>;
    
    struct MqttSubscription {
        String topic;
        MqttMessageCallback callback;
    };
    
    class MqttClient : public INetworkManager {
    public:
        explicit MqttClient(const farm::log::ILogger& logger, WiFiClient& wifiClient);
        ~MqttClient() override;
        
        bool initialize() override;
        NetworkResult connect() override;
        bool disconnect() override;
        bool isConnected() override;
        
        bool publish(const String& topic, const String& payload);
        bool subscribe(const String& topic, MqttMessageCallback callback);
        bool unsubscribe(const String& topic);
        void update();
        
    private:
        const farm::log::ILogger& logger;
        WiFiClient& wifiClient;
        PubSubClient mqttClient;
        std::vector<MqttSubscription> subscriptions;
        
        static void onMqttMessage(char* topic, byte* payload, unsigned int length);
        void handleMqttMessage(const String& topic, const String& payload);
    };
}
```

### Уровень периферийных устройств

#### Датчики (Sensors)

Все датчики реализуют общий интерфейс `ISensor`:

```cpp
namespace farm::io::sensors {
    class ISensor {
    public:
        virtual ~ISensor() = default;
        
        virtual bool initialize() = 0;
        virtual bool readValue(float& value) = 0;
        virtual const char* getSensorType() const = 0;
        virtual bool isConnected() const = 0;
    };
    
    // Примеры конкретных реализаций датчиков
    class TemperatureSensor : public ISensor {
        // Датчик температуры DS18B20
    };
    
    class HumiditySensor : public ISensor {
        // Датчик влажности почвы FC-28
    };
    
    class WaterLevelSensor : public ISensor {
        // Ультразвуковой датчик уровня воды HC-SR04
    };
    
    class WaterFlowSensor : public ISensor {
        // Расходомер воды YF-S401
    };
    
    class AirHumiditySensor : public ISensor {
        // Датчик влажности воздуха DHT22
    };
    
    class LightSensor : public ISensor {
        // Датчик освещенности KY-018
    };
}
```

#### Исполнительные устройства (Actuators)

Все исполнительные устройства реализуют общий интерфейс `IActuator`:

```cpp
namespace farm::io::actuators {
    class IActuator {
    public:
        virtual ~IActuator() = default;
        
        virtual bool initialize() = 0;
        virtual bool activate(float level = 1.0f) = 0;
        virtual bool deactivate() = 0;
        virtual bool isActive() const = 0;
        virtual float getCurrentLevel() const = 0;
        virtual const char* getActuatorType() const = 0;
    };
    
    // Примеры конкретных реализаций исполнительных устройств
    class WaterPump : public IActuator {
        // Мембранный насос R385
    };
    
    class GrowLight : public IActuator {
        // Фитолампа (LED-лента)
    };
    
    class HeatingLamp : public IActuator {
        // Лампа накаливания для подогрева
    };
}
```

### Уровень бизнес-логики

#### SystemStateManager
Менеджер состояния системы отслеживает текущее состояние всей системы и управляет переходами между состояниями.

```cpp
namespace farm::system {
    enum class SystemState {
        Initializing,
        Running,
        Error,
        Maintenance,
        PowerSaving,
        Shutdown
    };
    
    class SystemStateManager {
    public:
        SystemStateManager(farm::log::ILogger& logger);
        
        SystemState getCurrentState() const;
        void setCurrentState(SystemState newState);
        
        bool isError() const;
        void setError(const std::string& errorMessage);
        std::string getLastError() const;
        
    private:
        SystemState currentState;
        std::string lastErrorMessage;
        farm::log::ILogger& logger;
        
        void logStateChange(SystemState oldState, SystemState newState);
    };
}
```

#### EnvironmentController
Контролирует параметры окружающей среды, используя данные от датчиков и управляя исполнительными устройствами.

```cpp
namespace farm::control {
    struct EnvironmentParams {
        float airTemperature;
        float waterTemperature;
        float airHumidity;
        float soilMoisture;
        float lightLevel;
        float waterLevel;
    };
    
    class EnvironmentController {
    public:
        EnvironmentController(farm::log::ILogger& logger, farm::config::ConfigManager& config);
        
        void update();
        EnvironmentParams getCurrentParams() const;
        bool setTargetParams(const EnvironmentParams& params);
        
    private:
        EnvironmentParams currentParams;
        EnvironmentParams targetParams;
        farm::log::ILogger& logger;
        farm::config::ConfigManager& config;
        
        std::vector<farm::io::sensors::ISensor*> sensors;
        std::vector<farm::io::actuators::IActuator*> actuators;
        
        void updateSensorData();
        void adjustActuators();
    };
}
```

#### IrrigationController
Управляет системой полива растений.

```cpp
namespace farm::control {
    struct IrrigationSchedule {
        unsigned long startTime;
        unsigned long duration;
        float waterAmount;
    };
    
    class IrrigationController {
    public:
        IrrigationController(farm::log::ILogger& logger, farm::config::ConfigManager& config);
        
        void update();
        bool startIrrigation(float amount = 0, unsigned long duration = 0);
        bool stopIrrigation();
        bool isIrrigating() const;
        
        void setSchedule(const std::vector<IrrigationSchedule>& schedules);
        std::vector<IrrigationSchedule> getSchedule() const;
        
    private:
        bool irrigationActive;
        unsigned long irrigationStartTime;
        unsigned long irrigationDuration;
        float targetWaterAmount;
        float currentWaterUsed;
        
        farm::log::ILogger& logger;
        farm::config::ConfigManager& config;
        
        farm::io::sensors::WaterLevelSensor* waterLevelSensor;
        farm::io::sensors::WaterFlowSensor* waterFlowSensor;
        farm::io::sensors::HumiditySensor* soilMoistureSensor;
        farm::io::actuators::WaterPump* waterPump;
        
        std::vector<IrrigationSchedule> irrigationSchedules;
    };
}
```

#### LightingController
Управляет системой освещения для растений.

```cpp
namespace farm::control {
    struct LightingSchedule {
        unsigned long startTime;
        unsigned long duration;
        float intensity;
    };
    
    class LightingController {
    public:
        LightingController(farm::log::ILogger& logger, farm::config::ConfigManager& config);
        
        void update();
        bool setLightLevel(float level);
        float getLightLevel() const;
        
        void setSchedule(const std::vector<LightingSchedule>& schedules);
        std::vector<LightingSchedule> getSchedule() const;
        
    private:
        float currentLightLevel;
        
        farm::log::ILogger& logger;
        farm::config::ConfigManager& config;
        
        farm::io::sensors::LightSensor* lightSensor;
        farm::io::actuators::GrowLight* growLight;
        
        std::vector<LightingSchedule> lightingSchedules;
    };
}
```

#### HeatingController
Управляет системой контроля температуры.

```cpp
namespace farm::control {
    struct TemperatureRange {
        float minTemperature;
        float maxTemperature;
    };
    
    class HeatingController {
    public:
        HeatingController(farm::log::ILogger& logger, farm::config::ConfigManager& config);
        
        void update();
        bool setTargetTemperature(float temperature);
        float getTargetTemperature() const;
        float getCurrentTemperature() const;
        
    private:
        float targetTemperature;
        float currentTemperature;
        
        farm::log::ILogger& logger;
        farm::config::ConfigManager& config;
        
        farm::io::sensors::TemperatureSensor* temperatureSensor;
        farm::io::actuators::HeatingLamp* heatingLamp;
    };
}
```

## Диаграмма взаимодействия компонентов

```
+------------------+     +------------------+     +------------------+
|   Ядро системы   |     | Сетевой уровень  |     |  Уровень БЛ      |
+------------------+     +------------------+     +------------------+
| - IoPCore        |<--->| - INetworkManager|<--->| - StateManager   |
| - ConfigManager  |     | - WiFiManager    |     | - EnvController  |
| - Logger         |     | - MqttClient     |     | - IrrigController|
| - TaskScheduler  |     +------------------+     | - LightController|
+------------------+             ^                | - HeatController |
        ^                        |                +------------------+
        |                        v                        ^
        v                +------------------+             |
+------------------+     |  Перифер.устр.   |             |
|   Внешний API    |     +------------------+             |
+------------------+     | - ISensor        |<------------+
| - WebServer      |<----| - IActuator      |
| - RestAPI        |     +------------------+
| - MqttAPI        |
+------------------+
```

## Физические компоненты и их взаимодействие

### Датчики
1. **DS18B20** (датчик температуры)
   - Интерфейс: 1-Wire
   - Назначение: измерение температуры воды и воздуха
   - Класс: `TemperatureSensor`

2. **HC-SR04** (ультразвуковой датчик)
   - Интерфейс: цифровой (Trig/Echo пины)
   - Назначение: измерение уровня воды в резервуаре
   - Класс: `WaterLevelSensor`

3. **FC-28** (датчик влажности почвы)
   - Интерфейс: аналоговый
   - Назначение: измерение влажности почвы
   - Класс: `HumiditySensor`

4. **YF-S401** (расходомер воды)
   - Интерфейс: цифровой (импульсный)
   - Назначение: измерение расхода воды при поливе
   - Класс: `WaterFlowSensor`

5. **DHT22** (опционально) (датчик влажности воздуха)
   - Интерфейс: цифровой (однопроводной)
   - Назначение: измерение влажности воздуха
   - Класс: `AirHumiditySensor`

6. **KY-018** (опционально) (датчик освещенности)
   - Интерфейс: аналоговый
   - Назначение: измерение уровня освещенности
   - Класс: `LightSensor`

### Исполнительные устройства
1. **Мембранный насос R385**
   - Управление: через реле или MOSFET
   - Назначение: полив растений
   - Класс: `WaterPump`

2. **Фитолампа (LED-лента 12 В)**
   - Управление: через MOSFET с ШИМ
   - Назначение: обеспечение искусственного освещения для растений
   - Класс: `GrowLight`

3. **Лампа накаливания для подогрева**
   - Управление: через твердотельное реле (SSR)
   - Назначение: поддержание температуры
   - Класс: `HeatingLamp`

### Схема подключения

```
+------------+          +-------------+         +----------------+
| ESP32      |<-------->| Датчики     |         | Исполнительные |
| (Основной  |          |-------------|         | устройства     |
| контроллер)|          | DS18B20     |<------->|----------------|
+------------+          | HC-SR04     |         | Насос R385     |
      ^                 | FC-28       |         | Фитолампа LED  |
      |                 | YF-S401     |         | Лампа нагрева  |
      v                 | DHT22 (опц) |         +----------------+
+------------+          | KY-018 (опц)|                ^
| Управление |          +-------------+                |
| питанием   |<--------------------------------------->|
|------------|
| Реле модуль|
| MOSFET     |
| DC-DC      |
+------------+
```

## Заключение

Представленная архитектура IoP-Farm обеспечивает модульность, расширяемость и гибкость системы. Каждый компонент может быть разработан, тестирован и заменен независимо от других, что упрощает разработку и обслуживание системы. Логический и физический дизайн строго разделены, что позволяет изменять физические компоненты без серьезного изменения логики работы системы. 