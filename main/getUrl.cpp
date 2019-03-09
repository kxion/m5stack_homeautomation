#include <M5Stack.h>
#include <string.h>

#include "getUrl.h"

namespace mrdunk{

static char tag[] = "getUrl";

HttpRequest::HttpRequest(const char* url) {
  _url = strdup(url);

  // Create a curl handle
  handle = curl_easy_init();
  if (handle == NULL) {
    ESP_LOGD(tag, "Failed to create a curl handle");
    assert(0);
  }
  ESP_LOGD(tag, "Created a curl handle ...");

  curl_easy_setopt(handle, CURLOPT_URL, _url);
  curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&match);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeData);
}

HttpRequest::~HttpRequest() {
  free(_url);
  curl_easy_cleanup(handle);
}

uint8_t HttpRequest::get() {
  CURLcode res = curl_easy_perform(handle);
  if (res == CURLE_OK) {
    ESP_LOGD(tag, "curl_easy_perform() completed without incident.");
    if(match.string != nullptr) {
      return match.result;
    }
    return 0;
  }
  
  ESP_LOGW(tag, "curl_easy_perform failed: %s", curl_easy_strerror(res));
  assert(0);
  return 0;
}

size_t HttpRequest::writeData(void *buffer, size_t size, size_t nmemb, void *userp) {
	ESP_LOGD(tag, "writeData called: buffer=0x%lx, size=%d, nmemb=%d",
      (unsigned long)buffer, size, nmemb);
	ESP_LOGI(tag, "data>> %.*s", size*nmemb, (char *)buffer);

  struct Match* _match = (struct Match*)userp;
  if(_match->string != nullptr) {
    _match->result = (strncmp(_match->string, (char*)buffer, (size * nmemb) - 1) == 0);
  }
	return size * nmemb;
}

} // namespace mrdunk
