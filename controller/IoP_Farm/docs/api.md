# API документация IoP-Farm

## Содержание
1. [Введение](#введение)
2. [Форматы данных](#форматы-данных)
3. [MQTT API](#mqtt-api)
4. [REST API](#rest-api)
5. [Примеры использования](#примеры-использования)
6. [Коды ошибок](#коды-ошибок)
7. [Безопасность](#безопасность)

## Введение

API IoP-Farm обеспечивает программный интерфейс для мониторинга и управления системой "умной фермы". Доступны два основных варианта взаимодействия с системой:

1. **MQTT API** — для обмена сообщениями в реальном времени и взаимодействия с другими IoT-устройствами
2. **REST API** — для интеграции с веб-приложениями и другими внешними системами

Все API предоставляют возможности для:
- Получения данных с датчиков
- Управления исполнительными устройствами
- Настройки параметров системы
- Мониторинга состояния системы
- Управления расписанием задач

## Форматы данных

### JSON формат

Все API используют JSON в качестве основного формата обмена данными. Пример структуры данных датчиков:

```json
{
  "timestamp": 1635507245,
  "sensors": {
    "temperature": {
      "air": 22.5,
      "water": 21.0,
      "units": "C"
    },
    "humidity": {
      "soil": 65.3,
      "air": 45.7,
      "units": "%"
    },
    "water_level": {
      "value": 85.2,
      "units": "%"
    },
    "water_flow": {
      "value": 0.5,
      "units": "L/min"
    },
    "light": {
      "value": 432,
      "units": "lux"
    }
  }
}
```

### Статусы системы

Система может находиться в следующих состояниях:

| Статус | Описание |
|--------|----------|
| `initializing` | Система инициализируется |
| `running` | Система работает нормально |
| `error` | Система обнаружила ошибку |
| `maintenance` | Система в режиме обслуживания |
| `power_saving` | Система в режиме энергосбережения |
| `shutdown` | Система в процессе выключения |

## MQTT API

### Подключение

- **Брокер**: Адрес задается в конфигурационном файле (`confidential.h`)
- **Порт**: 1883 (несекретный) или 8883 (TLS)
- **Клиентский ID**: `iop-farm-{device_id}`
- **Пользователь/Пароль**: Настраиваются в конфигурационном файле (опционально)

### Топики

#### Топики публикации (устройство → сервер)

| Топик | Описание | Формат | Частота |
|-------|----------|--------|---------|
| `farm/{device_id}/state` | Текущее состояние системы | JSON | При изменении |
| `farm/{device_id}/sensors` | Данные всех датчиков | JSON | По расписанию |
| `farm/{device_id}/sensors/temperature` | Данные датчиков температуры | JSON | По расписанию |
| `farm/{device_id}/sensors/humidity` | Данные датчиков влажности | JSON | По расписанию |
| `farm/{device_id}/sensors/water_level` | Данные датчика уровня воды | JSON | По расписанию |
| `farm/{device_id}/sensors/water_flow` | Данные расходомера воды | JSON | При поливе |
| `farm/{device_id}/sensors/light` | Данные датчика освещенности | JSON | По расписанию |
| `farm/{device_id}/actuators` | Статус всех исполнительных устройств | JSON | При изменении |
| `farm/{device_id}/logs` | Логи системы | JSON | При событиях |

#### Топики подписки (сервер → устройство)

| Топик | Описание | Формат | Пример |
|-------|----------|--------|--------|
| `farm/{device_id}/cmd/system` | Управление системой | JSON | `{"action": "restart"}` |
| `farm/{device_id}/cmd/pump` | Управление насосом | JSON | `{"action": "start", "duration": 30}` |
| `farm/{device_id}/cmd/light` | Управление освещением | JSON | `{"action": "set", "level": 75}` |
| `farm/{device_id}/cmd/heating` | Управление нагревателем | JSON | `{"action": "set", "value": true}` |
| `farm/{device_id}/config` | Обновление конфигурации | JSON | `{"schedule": {...}}` |

### Примеры сообщений

#### Статус системы

```json
{
  "status": "running",
  "uptime": 345600,
  "memory": {
    "free": 124560,
    "total": 327680
  },
  "wifi": {
    "connected": true,
    "signal": -68,
    "ip": "192.168.1.105"
  }
}
```

#### Управление насосом

Запрос:
```json
{
  "action": "start",
  "duration": 60,
  "request_id": "abcd1234"
}
```

Ответ (в топике `farm/{device_id}/actuators`):
```json
{
  "pump": {
    "status": "active",
    "level": 100,
    "duration_left": 58,
    "request_id": "abcd1234"
  },
  "light": {
    "status": "active",
    "level": 75
  },
  "heating": {
    "status": "inactive",
    "level": 0
  }
}
```

#### Установка расписания

```json
{
  "irrigation": [
    {
      "start_time": "07:00",
      "duration": 300,
      "days": [1, 3, 5]
    },
    {
      "start_time": "19:00",
      "duration": 300,
      "days": [1, 3, 5]
    }
  ],
  "lighting": [
    {
      "start_time": "06:00",
      "end_time": "22:00",
      "level": 80,
      "days": [1, 2, 3, 4, 5, 6, 7]
    }
  ]
}
```

## REST API

REST API доступен при включенном веб-сервере по адресу `http://{устройство_ip}/api/`

### Аутентификация

API использует базовую HTTP-аутентификацию. Учетные данные настраиваются в веб-интерфейсе или конфигурационном файле.

Заголовок для аутентификации:
```
Authorization: Basic {base64(username:password)}
```

### Методы API

#### Получение данных датчиков

- **URL**: `/api/sensors`
- **Метод**: `GET`
- **Параметры**:
  - `type` - тип датчика (опционально, например `temperature`)
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `401 Unauthorized` - неверные учетные данные
  - `404 Not Found` - запрашиваемый датчик не найден

#### Получение статуса исполнительных устройств

- **URL**: `/api/actuators`
- **Метод**: `GET`
- **Параметры**:
  - `type` - тип устройства (опционально, например `pump`)
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `401 Unauthorized` - неверные учетные данные
  - `404 Not Found` - запрашиваемое устройство не найдено

#### Управление насосом

- **URL**: `/api/actuators/pump`
- **Метод**: `POST`
- **Данные**:
  ```json
  {
    "action": "start",
    "duration": 60
  }
  ```
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `400 Bad Request` - неверные параметры
  - `401 Unauthorized` - неверные учетные данные
  - `409 Conflict` - конфликт с текущим состоянием

#### Управление освещением

- **URL**: `/api/actuators/light`
- **Метод**: `POST`
- **Данные**:
  ```json
  {
    "action": "set",
    "level": 75
  }
  ```
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `400 Bad Request` - неверные параметры
  - `401 Unauthorized` - неверные учетные данные

#### Получение текущего расписания

- **URL**: `/api/schedule`
- **Метод**: `GET`
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `401 Unauthorized` - неверные учетные данные

#### Установка расписания

- **URL**: `/api/schedule`
- **Метод**: `POST`
- **Данные**: JSON с расписанием (см. пример в MQTT API)
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `400 Bad Request` - неверные параметры
  - `401 Unauthorized` - неверные учетные данные

#### Получение статуса системы

- **URL**: `/api/system/status`
- **Метод**: `GET`
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `401 Unauthorized` - неверные учетные данные

#### Управление системой

- **URL**: `/api/system`
- **Метод**: `POST`
- **Данные**:
  ```json
  {
    "action": "restart"
  }
  ```
- **Возможные действия**:
  - `restart` - перезагрузка системы
  - `maintenance` - переход в режим обслуживания
  - `update` - обновление прошивки (если доступно)
- **Коды ответов**:
  - `200 OK` - успешное выполнение
  - `400 Bad Request` - неверные параметры
  - `401 Unauthorized` - неверные учетные данные
  - `403 Forbidden` - действие запрещено

## Примеры использования

### Пример использования MQTT API

В приведенном ниже примере показано, как подписаться на данные датчиков и отправить команду управления насосом с использованием библиотеки Mosquitto MQTT.

```python
import paho.mqtt.client as mqtt
import json
import time

# Настройки подключения
broker = "farm.example.com"
port = 1883
device_id = "farm-001"
username = "farm_user"
password = "secret"

# Функция обработки полученных сообщений
def on_message(client, userdata, msg):
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")
    
    # Парсинг JSON
    if msg.topic == f"farm/{device_id}/sensors":
        sensor_data = json.loads(msg.payload.decode())
        print(f"Current soil humidity: {sensor_data['humidity']['soil']}%")

# Инициализация клиента
client = mqtt.Client(f"python-client-{int(time.time())}")
client.username_pw_set(username, password)
client.on_message = on_message

# Подключение к брокеру
client.connect(broker, port, 60)

# Подписка на топики
client.subscribe(f"farm/{device_id}/sensors")
client.subscribe(f"farm/{device_id}/state")
client.subscribe(f"farm/{device_id}/actuators")

# Запуск обработки сообщений в фоновом режиме
client.loop_start()

# Отправка команды включения насоса на 30 секунд
client.publish(f"farm/{device_id}/cmd/pump", json.dumps({
    "action": "start",
    "duration": 30,
    "request_id": "test-request-001"
}))

# Ожидание обработки сообщений
time.sleep(60)

# Завершение работы
client.loop_stop()
client.disconnect()
```

### Пример использования REST API

В приведенном ниже примере показано, как получить данные датчиков и управлять насосом с использованием библиотеки Requests.

```python
import requests
import json
import base64

# Настройки подключения
base_url = "http://192.168.1.105/api"
username = "admin"
password = "admin"

# Базовая аутентификация
auth_header = {
    "Authorization": "Basic " + base64.b64encode(f"{username}:{password}".encode()).decode()
}

# Получение данных всех датчиков
response = requests.get(f"{base_url}/sensors", headers=auth_header)

if response.status_code == 200:
    sensors_data = response.json()
    print(f"Current soil humidity: {sensors_data['humidity']['soil']}%")
    print(f"Current air temperature: {sensors_data['temperature']['air']}°C")
else:
    print(f"Error: {response.status_code} - {response.text}")

# Включение насоса на 30 секунд
pump_command = {
    "action": "start",
    "duration": 30
}

response = requests.post(
    f"{base_url}/actuators/pump",
    headers={**auth_header, "Content-Type": "application/json"},
    data=json.dumps(pump_command)
)

if response.status_code == 200:
    result = response.json()
    print(f"Pump control result: {result['status']}")
else:
    print(f"Error: {response.status_code} - {response.text}")
```

## Коды ошибок

### MQTT API Ошибки

Ошибки MQTT отправляются в топик `farm/{device_id}/errors` в формате JSON:

```json
{
  "timestamp": 1635507300,
  "code": "ERR_INVALID_PARAMS",
  "message": "Invalid parameters for pump control",
  "details": {
    "request_id": "abcd1234",
    "topic": "farm/farm-001/cmd/pump"
  }
}
```

### Основные коды ошибок

| Код ошибки | Описание |
|------------|----------|
| `ERR_INVALID_PARAMS` | Неверные параметры запроса |
| `ERR_AUTH_FAILED` | Ошибка аутентификации |
| `ERR_NOT_FOUND` | Запрашиваемый ресурс не найден |
| `ERR_FORBIDDEN` | Доступ запрещен |
| `ERR_CONFLICT` | Конфликт с текущим состоянием |
| `ERR_INTERNAL` | Внутренняя ошибка сервера |
| `ERR_TIMEOUT` | Превышено время ожидания ответа |
| `ERR_NOT_IMPLEMENTED` | Функция не реализована |

## Безопасность

### Рекомендации по безопасности

1. **Изменение учетных данных по умолчанию**
   - Обязательно измените пароли по умолчанию при первом использовании

2. **Использование TLS для MQTT**
   - Для повышения безопасности рекомендуется использовать подключение к MQTT брокеру через TLS (порт 8883)

3. **Ограничение доступа**
   - Ограничьте доступ к API только с доверенных IP-адресов
   - Используйте VPN для удаленного доступа

4. **Регулярное обновление**
   - Следите за обновлениями прошивки и устанавливайте их для закрытия потенциальных уязвимостей

### Уровни доступа

В системе предусмотрены следующие уровни доступа:

| Уровень | Описание | Доступные операции |
|---------|----------|-------------------|
| Администратор | Полный доступ ко всем функциям | Чтение/запись всех данных, конфигурация, обновление системы |
| Оператор | Управление работой системы | Чтение всех данных, управление исполнительными устройствами |
| Монитор | Только мониторинг | Чтение данных датчиков и состояния системы |

### Ограничение API

По умолчанию API имеет следующие ограничения:
- Не более 60 запросов в минуту для REST API
- Не более 10 сообщений в секунду для MQTT API
- Максимальный размер сообщения: 8 КБ 