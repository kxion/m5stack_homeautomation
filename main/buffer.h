#ifndef DUNK__ESP32__BUFFER_H
#define DUNK__ESP32__BUFFER_H

extern "C" {
#include "flatmap.h"
}

void mqttBuffer_put(const char* topic, const int topicLen, const char* data, const int dataLen);
void mqttBuffer_getMatching(const char* match, MapInfo* mapCopy);
void mqttBuffer_dispalyAll(MapInfo* _mapInfo);
void mqttBuffer_dispalyAll();

#endif  // DUNK__ESP32__BUFFER_H
