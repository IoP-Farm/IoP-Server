#include "NetworkManager.h"
#include "Config.h"
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);
unsigned long lastUpdate = 0;

void MyNetworkManager::connectWiFi() {
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

void MyNetworkManager::connectMQTT() {
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

void MyNetworkManager::sendData(const String& payload) {
  if (!mqttClient.connected()) return;
  mqttClient.publish(config.mqtt.topicData, payload.c_str());
}

void MyNetworkManager::mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Received command: " + message);
}