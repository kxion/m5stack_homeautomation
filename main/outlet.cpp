#include <M5Stack.h>
#include <stdint.h>
#include <string.h>

#include "outlet.h"
#include "mqtt.h"

namespace mrdunk{

Outlet::Outlet(const char* topicBase, const boolean defaultState) :
    _topicBase(strdup(topicBase)), _defaultState(defaultState) {
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
  mqtt_subscribe(topic);
}

OutletKankun::OutletKankun(const char* topicBase, const boolean defaultState) :
    Outlet(topicBase, defaultState) {
  _httpRequestState = new mrdunk::HttpRequest("http://192.168.192.8/cgi-bin/relay.cgi?state");
  _httpRequestState->match = {.string = "ON", .result = 0};
}

OutletKankun::~OutletKankun() {
  free(_httpRequestState);
}

void OutletKankun::update() {
  _httpRequestState->get();
  char payload[30] = "";
  snprintf(payload, 30, "state:%i", _httpRequestState->match.result);
  mqtt_publish(_topicBase, payload);
}

boolean OutletKankun::getState() {
  update();
  return _httpRequestState->match.result;
}

void OutletKankun::setState(const boolean state) {
}

} // namespace mrdunk

