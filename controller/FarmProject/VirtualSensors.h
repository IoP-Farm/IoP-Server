#ifndef VIRTUAL_SENSORS_H
#define VIRTUAL_SENSORS_H

class VirtualSensors {
public:
  float readTemperature();
  float readHumidity();
  float readWaterLevel();
};

#endif