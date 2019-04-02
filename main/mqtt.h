#ifndef DUNK__ESP32__MQTT_H
#define DUNK__ESP32__MQTT_H

#include <stdint.h>
#include <string.h>

void wifi_init(void);
void mqtt_app_start(void);
uint8_t mqtt_publish(const char* topic, const char* message);
uint8_t mqtt_subscribe(const char* topic);

// This method is in the mqttHelpers.cpp file.
uint8_t matchTopic(const char* match, const char* topic);

#endif  // DUNK__ESP32__MQTT_H
