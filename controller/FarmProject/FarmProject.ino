#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== Конфигурация =====
struct Config {
  struct {
    const char* ssid = "&Dolgopniki_5G";    // Замените на свои данные
    const char* password = "KSP_kr0t0vuxa";  // Замените на свои данные
  } wifi;
  
  struct {
    const char* server = "103.137.250.154";
    int port = 1883;
    const char* clientId = "farm_001";
    const char* topicData = "farm/data";
    const char* topicControl = "farm/control";
  } mqtt;
  
  unsigned long updateInterval = 10000;
};

// ===== Глобальные объекты =====
Config config;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastUpdate = 0;

// ===== Имитация датчиков =====
class VirtualSensors {
private:
  void initRandom() {
    static bool seeded = false;
    if (!seeded) {
      randomSeed(analogRead(36));  // Используем GPIO36 (Analog A0)
      seeded = true;
    }
  }

public:
  float readTemperature() {
    initRandom();
    return random(180, 301) / 10.0f;
  }

  float readHumidity() {
    initRandom();
    return random(400, 801) / 10.0f;
  }

  float readWaterLevel() {
    initRandom();
    return random(500, 1001) / 10.0f;
  }
};

// ===== Сетевые функции =====
class MyNetworkManager {
public:
  void connectWiFi() {
    if (isWiFiConnected()) return;
    
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    
    Serial.print("\nПодключение к ");
    Serial.println(config.wifi.ssid);
    
    WiFi.begin(config.wifi.ssid, config.wifi.password);
    
    unsigned long start = millis();
    while (!isWiFiConnected() && millis() - start < 20000) {
      delay(500);
      Serial.print(".");
    }
    
    if (isWiFiConnected()) {
      Serial.print("\nПодключено! IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nОшибка подключения!");
      WiFi.disconnect(true);
    }
  }

  void manageMQTT() {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    }
    mqttClient.loop();
  }

  void sendData(const String& payload) {
    if (mqttClient.connected()) {
      mqttClient.publish(config.mqtt.topicData, payload.c_str());
    }
  }

private:
  bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
  }

  void reconnectMQTT() {
    static unsigned long lastTry;
    
    if (millis() - lastTry < 5000) return;
    lastTry = millis();
    
    Serial.println("Подключение к MQTT...");
    
    mqttClient.setServer(config.mqtt.server, config.mqtt.port);
    mqttClient.setCallback([](char* topic, byte* payload, unsigned int len) {
      String msg;
      for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
      Serial.printf("Команда [%s]: %s\n", topic, msg.c_str());
    });
    
    if (mqttClient.connect(config.mqtt.clientId)) {
      mqttClient.subscribe(config.mqtt.topicControl);
      Serial.println("MQTT подключен!");
    } else {
      Serial.println("Ошибка MQTT!");
    }
  }
};

// ===== Экземпляры классов =====
MyNetworkManager network;
VirtualSensors sensors;

void setup() {
  Serial.begin(115200);
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  network.connectWiFi();
}

void loop() {
  network.connectWiFi();
  network.manageMQTT();

  if (millis() - lastUpdate >= config.updateInterval) {
    StaticJsonDocument<200> doc;
    doc["device_id"] = config.mqtt.clientId;
    doc["temperature"] = sensors.readTemperature();
    doc["humidity"] = sensors.readHumidity();
    doc["water_level"] = sensors.readWaterLevel();

    String payload;
    serializeJson(doc, payload);
    network.sendData(payload);
    
    lastUpdate = millis();
    Serial.println("Отправлено: " + payload);
  }
  delay(100);
}