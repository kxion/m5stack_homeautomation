#include <M5Stack.h>
#include <stdint.h>
#include <string.h>
#include <freertos/semphr.h>

#include "outlet.h"
#include "mqtt.h"
#include "buffer.h"
extern "C" {
#include "flatmap.h"
}

namespace mrdunk{

Outlet::Outlet(const char* topicBase, const boolean defaultState) :
    _topicBase(strdup(topicBase)), _defaultState(defaultState), lastPublishTime(0) {
}

Outlet::~Outlet() {
  free(_topicBase);
  for(uint8_t p = 0; p < MAX_PRIORITY; p++) {
    for(std::vector<char*>::iterator it = topicsAtPriority[p].begin();
        it != topicsAtPriority[p].end();
        ++it) {
      free(*it);
    }
  }
}

void Outlet::addControlSubscription(const char* topic, const uint8_t priority) {
  if(priority >= MAX_PRIORITY) {
    ESP_LOGW("outlet", "Out of range topic priority: %i", priority);
    return;
  }
  for(std::vector<char*>::iterator it = topicsAtPriority[priority].begin();
      it != topicsAtPriority[priority].end();
      ++it) {
    if(strcmp(*it, topic) == 0) {
      // Already have this topic.
      return;
    }
  }
  topicsAtPriority[priority].push_back(strdup(topic));
  assert(mqtt_subscribe(topic));
}

void Outlet::publish(const boolean state) {
  if(lastPublishState != state ||
      lastPublishTime == 0 ||
      lastPublishTime + PUBLISH_INTERVAL < millis()) {
    lastPublishState = state;
    lastPublishTime = millis();
    char payload[30] = "";
    snprintf(payload, 30, "state:%i", state);
    mqtt_publish(_topicBase, payload);
  }
}

OutletKankun::OutletKankun(const char* topicBase, const boolean defaultState) :
    Outlet(topicBase, defaultState) {
  _httpRequestState = new mrdunk::HttpRequest("http://192.168.192.8/cgi-bin/relay.cgi?state");
  _httpRequestState->match = {.string = "ON", .result = 0};

  mutex = xSemaphoreCreateMutex();
  assert(mutex && "Unable to create mutex");
}

OutletKankun::~OutletKankun() {
  free(_httpRequestState);
}

void OutletKankun::update() {
  ESP_LOGD("outletKankun", "OutletKankun::update()");
  
  _httpRequestState->get();
  publish(_httpRequestState->match.result);
  
  ESP_LOGD("!outletKankun", "OutletKankun::update()");
}

boolean OutletKankun::getState() {
  xSemaphoreTake(mutex, portMAX_DELAY);
  if(lastPublishTime == 0 || lastPublishTime + 5000 < millis()) {
    update();
  }
  xSemaphoreGive(mutex);

  // Get a snapshot of the topics we care about from buffer.
  MapInfo mapCopy;
  mqttBuffer_getMatching(_topicBase, &mapCopy);
    
  size_t switchCount = flatmap_getEntryCount(&mapCopy);
  if(switchCount == 0) {
    ESP_LOGE("outletKankun", "No outlet found.");
    flatmap_free(&mapCopy);
    return 0;
  }

  // Presume the first returned entry is the one we want.
  Entry* entry = flatmap_getByIndex(&mapCopy, 0);
  Entry subEntry = {
    .key = (char*)"state",
    .content = nullptr
  };
  Entry* result = flatmap_get((MapInfo*)entry->content, &subEntry);
  uint32_t state = strtod((char*)result->content, NULL);

  flatmap_free(&mapCopy);
  return state;
}

void OutletKankun::setState(const boolean state) {
  // TODO
}

} // namespace mrdunk

