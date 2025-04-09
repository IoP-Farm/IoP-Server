# server.py
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    print("Connected with code", rc)
    client.subscribe("esp32/sensors")

def on_message(client, userdata, msg):
    print(f"Topic: {msg.topic}\nMessage: {msg.payload.decode()}")
    
    # Отправка команды обратно
    if msg.topic == "esp32/sensors":
        client.publish("esp32/command", "DATA_RECEIVED")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()
