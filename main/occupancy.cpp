#include <M5Stack.h>
#include <stdio.h>
#include <stdint.h>

#include "mqtt.h"
#include "config.h"

#define OCCUPANCY_VIA_BUTTON GLOBAL_ID "/occupancy/switchChange/" DEVICE_NAME

static int32_t timeout = VALID_TIME_BUTTON_PRESS;
static int32_t timeoutLastSet = -VALID_TIME_BUTTON_PRESS;
static uint8_t currentOccupancy = 1;

void occupancy_set() {
  timeoutLastSet = millis();
  currentOccupancy = 1;
  char buffer[50];
  snprintf(buffer, 50, "state:true,trueFor:%i", VALID_TIME_BUTTON_PRESS);
  mqtt_publish(OCCUPANCY_VIA_BUTTON, buffer);
}

void occupancy_update() {
  if(M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
    occupancy_set();
  }

  if(((timeoutLastSet + timeout) < millis()) && currentOccupancy) {
    currentOccupancy = 0;
    mqtt_publish(OCCUPANCY_VIA_BUTTON, "state:false,trueFor:undefined");
  }
}
