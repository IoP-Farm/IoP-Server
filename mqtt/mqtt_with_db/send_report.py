import sqlite3
import requests
import time
from datetime import datetime

# Настройки
DB_NAME = "mqtt_data.db"
REMOTE_SERVER_URL = "http://192.168.1.100:5000/receive_report"  # Замените на ваш IP/порт
INTERVAL_MINUTES = 15

def get_last_5_records():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    cursor.execute('''
        SELECT device_id, temperature, humidity, water_level, timestamp
        FROM farm_data
        ORDER BY timestamp DESC
        LIMIT 5
    ''')
    records = cursor.fetchall()
    conn.close()
    return [
        {
            "device_id": r[0],
            "temperature": r[1],
            "humidity": r[2],
            "water_level": r[3],
            "timestamp": r[4]
        }
        for r in records
    ]

def send_report():
    while True:
        try:
            records = get_last_5_records()
            if records:
                response = requests.post(REMOTE_SERVER_URL, json={"last_5_messages": records})
                print(f"{datetime.now()}: Отчёт отправлен. Ответ сервера: {response.text}")
            else:
                print("Нет данных для отправки.")
        except Exception as e:
            print(f"Ошибка: {e}")
        
        time.sleep(INTERVAL_MINUTES * 60)  # Пауза 15 минут

if __name__ == "__main__":
    print(f"Отправка отчётов каждые {INTERVAL_MINUTES} минут...")
    send_report()
