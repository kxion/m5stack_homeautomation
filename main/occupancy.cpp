#include <M5Stack.h>
#include <stdio.h>
#include <stdint.h>

#include "mqtt.h"
#include "config.h"
//#include "getUrl.h"

#define OCCUPANCY_VIA_BUTTON GLOBAL_ID "/occupancy/switchChange/" DEVICE_NAME

static int32_t timeout = VALID_TIME_BUTTON_PRESS;
static int32_t timeoutLastSet = -VALID_TIME_BUTTON_PRESS;
static uint8_t currentOccupancy = 1;
//static mrdunk::HttpRequest* httpRequest;

void occupancy_set() {
  timeoutLastSet = millis();
  currentOccupancy = 1;
  char buffer[50];
  snprintf(buffer, 50, "state:true,trueFor:%i", VALID_TIME_BUTTON_PRESS);
  mqtt_publish(OCCUPANCY_VIA_BUTTON, buffer);

  /*if(httpRequest == nullptr) {
    httpRequest = new mrdunk::HttpRequest(
        "https://script.google.com/macros/s/"
        "AKfycbxnv6e2OrhUPW0lw05jHF-sX64VJRyBf1HdgUxjxdtR18rWHVA/exec");
    httpRequest->curlTest();
  }
  httpRequest->setPostFields("user=unknown&reason=switchChange/heatingControler&topic="
                            OCCUPANCY_VIA_BUTTON);
  httpRequest->get(); */
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
