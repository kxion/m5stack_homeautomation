#include <M5Stack.h>
#include <stdlib.h>

#include "mqtt.h"
#include "buffer.h"
extern "C" {
#include "flatmap.h"
}
#include "temparature.h"

namespace mrdunk {
  Temperature::Temperature(const char* zone) {
    _zone = strdup(zone);
    const size_t topicLen = strlen(TOPIC_TEMPERATURES) + strlen(_zone) + 4;
    char topic[topicLen];
    snprintf(topic, topicLen, "%s/%s/#", TOPIC_TEMPERATURES, _zone);
    mqtt_subscribe(topic);
  }

  Temperature::~Temperature() {
    free(_zone);
  }

  float Temperature::average() {
    // Get a snapshot of the topics we care about.
    MapInfo mapCopy;
    mqttBuffer_getMatching("7abr/sensor/temperature/#", &mapCopy);

    // Iterate through them, calculating an average of any temperatures.
    float totalTemperature = 0;
    size_t temperatureCount = flatmap_getEntryCount(&mapCopy);
    for(size_t index = 0; index < temperatureCount; index++) {
      Entry* entry = flatmap_getByIndex(&mapCopy, index);
      Entry subEntry = {
        .key = (char*)"temperature",
        .content =nullptr
      };
      Entry* result = flatmap_get((MapInfo*)entry->content, &subEntry);
      totalTemperature += strtod((char*)result->content, NULL);
    }
    flatmap_free(&mapCopy);
    printf("Free heap: %i\n", xPortGetFreeHeapSize());
    return totalTemperature / temperatureCount;
  }
}

