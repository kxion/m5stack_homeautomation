#ifndef DUNK__OUTLET__GETURL_H
#define DUNK__OUTLET__GETURL_H

#include <vector>
#include "getUrl.h"

namespace mrdunk {

#define MAX_PRIORITY 10

class Outlet {
 public:
  Outlet(const char* topicBase, const boolean defaultState);
  ~Outlet();
  void addControlSubscription(const char* topic, const uint8_t priority);
  virtual void update()=0;
  virtual boolean getState()=0;
 protected:
  char* _topicBase;
  const boolean _defaultState;
  virtual void setState(const boolean state)=0;
  std::vector<char*> topicsAtPriority[MAX_PRIORITY];
};

class OutletKankun: public Outlet {
 public:
  OutletKankun(const char* topicBase, const boolean defaultState);
  ~OutletKankun();
  void update();
  boolean getState();
 private:
  HttpRequest* _httpRequestState;
  void setState(const boolean state);
};

} // namespace mrdunk

#endif  // DUNK__OUTLET__GETURL_H

