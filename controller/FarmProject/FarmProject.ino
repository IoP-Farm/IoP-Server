#include "Config.h"
#include "VirtualSensors.h"
#include "NetworkManager.h"

Config config;
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