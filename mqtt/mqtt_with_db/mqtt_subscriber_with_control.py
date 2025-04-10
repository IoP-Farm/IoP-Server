import paho.mqtt.client as mqtt
import json
import sqlite3
import random
import time

# Настройки
DB_NAME = "mqtt_data.db"
MQTT_BROKER = "127.0.0.1"  # Или IP вашего брокера
MQTT_TOPIC_DATA = "farm/data"
MQTT_TOPIC_CONTROL = "farm/control"  # Топик для отправки команд на ESP32

# Инициализация БД
def init_db():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS farm_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT,
            temperature REAL,
            humidity REAL,
            water_level REAL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    conn.commit()
    conn.close()

# Сохранение данных в SQLite
def save_to_db(data):
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO farm_data (device_id, temperature, humidity, water_level)
        VALUES (?, ?, ?, ?)
    ''', (data["device_id"], data["temperature"], data["humidity"], data["water_level"]))
    conn.commit()
    conn.close()

# Отправка данных в топик farm/control
def send_control_data(client):
    while True:
        data = {
            "temperature": random.randint(20, 30),  # Пример: 20-30°C
            "brightness": random.randint(0, 100)    # Пример: 0-100%
        }
        client.publish(MQTT_TOPIC_CONTROL, json.dumps(data))
        print(f"Отправлено в farm/control: {data}")
        time.sleep(15)  # Отправка каждые 15 секунд (для теста)

# Обработка входящих сообщений
def on_message(client, userdata, msg):
    try:
        if msg.topic == MQTT_TOPIC_DATA:
            data = json.loads(msg.payload.decode())
            print(f"Данные с фермы: {data}")
            save_to_db(data)
    except Exception as e:
        print(f"Ошибка: {e}")

def run_mqtt_client():
    init_db()
    client = mqtt.Client()
    client.on_message = on_message
    
    client.connect(MQTT_BROKER, 1883, 60)
    client.subscribe(MQTT_TOPIC_DATA)
    client.loop_start()
    
    print("Сервер запущен. Отправка данных в farm/control...")
    send_control_data(client)  # Запуск цикла отправки

if __name__ == "__main__":
    run_mqtt_client()
