#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "Config.h"

class MyNetworkManager {
public:
  void connectWiFi();
  void connectMQTT();
  void sendData(const String& payload);

private:
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

#endif