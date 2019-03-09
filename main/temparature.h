#ifndef DUNK__ESP32__TEMPERATURE_H
#define DUNK__ESP32__TEMPERATURE_H

namespace mrdunk {

#define TOPIC_TEMPERATURES "7abr/sensor/temperature"

class Temperature {
 public:
  Temperature(const char* zone);
  ~Temperature();
  float average();
 private:
  char* _zone;
};

}
#endif  // DUNK__ESP32__TEMPERATURE_H
