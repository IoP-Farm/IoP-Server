#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

struct Config {
  struct {
    const char* ssid = "320_Wi-Fi5";
    const char* password = "cat4rsys";
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

extern Config config;
extern PubSubClient mqttClient;
extern unsigned long lastUpdate;

#endif