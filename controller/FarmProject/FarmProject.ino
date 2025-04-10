#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== Конфигурация =====
struct Config {
  struct {
    //const char* ssid = "320_Wi-Fi5";
    //const char* password = "cat4rsys";

    const char* ssid = "&Dolgopniki";
    const char* password = "KSP_kr0t0vuxa";
  } wifi;
  
  struct {
    const char* server = "103.137.250.154"; // Адрес MQTT-брокера
    int port = 1883;
    const char* clientId = "farm_001";
    const char* topicData = "farm/data";
    const char* topicControl = "farm/control";
  } mqtt;
  
  unsigned long updateInterval = 10000; // Интервал отправки данных (мс)
};

// ===== Глобальные объекты =====
Config config;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastUpdate = 0;

// ===== Имитация датчиков =====
class VirtualSensors {
private:
  // Инициализация генератора случайных чисел
  void initRandom() {
    static bool seeded = false;
    if (!seeded) {
      randomSeed(analogRead(A0));  // Используем "шум" с аналогового пина
      seeded = true;
    }
  }

public:
  float readTemperature() {
    initRandom();
    return random(180, 301) / 10.0f;  // 18.0°C - 30.0°C
  }

  float readHumidity() {
    initRandom();
    return random(400, 801) / 10.0f;  // 40.0% - 80.0%
  }

  float readWaterLevel() {
    initRandom();
    return random(500, 1001) / 10.0f;  // 50.0% - 100.0%
  }
};

// ===== Сетевые функции =====
class MyNetworkManager {
public:
  void connectWiFi() {
    if (WiFi.status() == WL_CONNECTED) return;
    
    Serial.println("Connecting to WiFi...");
    WiFi.begin(config.wifi.ssid, config.wifi.password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts++ < 20) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected! IP: " + WiFi.localIP());
    } else {
      Serial.println("\nWiFi connection failed!");
    }
  }

  void connectMQTT() {
    if (mqttClient.connected()) return;

    mqttClient.setServer(config.mqtt.server, config.mqtt.port);
    mqttClient.setCallback(mqttCallback);

    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(config.mqtt.clientId)) {
      Serial.println("MQTT connected!");
      mqttClient.subscribe(config.mqtt.topicControl);
    } else {
      Serial.println("MQTT connection failed!");
    }
  }

  void sendData(const String& payload) {
    if (!mqttClient.connected()) return;
    
    mqttClient.publish(config.mqtt.topicData, payload.c_str());
    Serial.println("Data sent: " + payload);
  }

private:
  static void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    Serial.println("Received command: " + message);
    // Здесь обрабатываем команды
  }
};

// ===== Экземпляры классов =====
MyNetworkManager network;
VirtualSensors sensors;

void setup() {
  Serial.begin(115200);
  network.connectWiFi();
  network.connectMQTT();
}

void loop() {
  network.connectWiFi();
  network.connectMQTT();
  mqttClient.loop();

  // Отправка данных по таймеру
  if (millis() - lastUpdate >= config.updateInterval) {
    DynamicJsonDocument doc(256);
    doc["device_id"] = config.mqtt.clientId;
    doc["temperature"] = sensors.readTemperature();
    doc["humidity"] = sensors.readHumidity();
    doc["water_level"] = sensors.readWaterLevel();

    String payload;
    serializeJson(doc, payload);
    network.sendData(payload);
    
    lastUpdate = millis();
  }
}