# server.py
import paho.mqtt.client as mqtt
import json

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT! Code:", rc)
    client.subscribe("farm/data")  # Подписываемся на данные фермы
    client.subscribe("farm/control")  # Для двусторонней связи

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        if msg.topic == "farm/data":
            print("\nДанные с фермы:")
            print(f"Устройство: {payload['device_id']}")
            print(f"Температура: {payload['temperature']}°C")
            print(f"Влажность: {payload['humidity']}%")
            print(f"Уровень воды: {payload['water_level']}%")
        elif msg.topic == "farm/control":
            print("Ответ на команду:", payload)
    except Exception as e:
        print("Ошибка:", e)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Подключение к брокеру
client.connect("127.0.0.1", 1883, 60)

# Запуск цикла обработки сообщений
client.loop_start()

# Простой интерфейс для отправки команд
print("Сервер запущен. Введите команду (или 'exit' для выхода):")
while True:
    cmd = input("> ")
    if cmd.lower() == "exit":
        break
    # Отправка команды на ферму
    client.publish("farm/control", cmd)

client.loop_stop()
client.disconnect()
