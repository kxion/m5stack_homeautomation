#include <M5Stack.h>
#include <string.h>

#include "getUrl.h"

namespace mrdunk{

uint8_t HttpRequest::globalInit = 0;
static char tag[] = "getUrl";

HttpRequest::HttpRequest(const char* url) :  match({"", 0}) {
  ESP_LOGD(tag, "HttpRequest");
  _url = strdup(url);

  // Create a curl handle
  if(globalInit == 0) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
  }
  globalInit++;

  handle = curl_easy_init();
  if (handle == NULL) {
    ESP_LOGD(tag, "Failed to create a curl handle");
    assert(0);
  }
  ESP_LOGD(tag, "Created a curl handle ...");

  //curl_easy_setopt(handle, CURLOPT_URL, _url);
  //curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
  //curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&match);
  //curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeData);
}

void HttpRequest::curlTest() {
  curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
	ESP_LOGI(tag, "Curl version info");
	ESP_LOGI(tag, "version: %s - %d", data->version, data->version_num);
  if (data->features & CURL_VERSION_IPV6) {
		ESP_LOGI(tag, "- IP V6 supported");
	} else {
		ESP_LOGI(tag, "- IP V6 NOT supported");
	}
	if (data->features & CURL_VERSION_SSL) {
		ESP_LOGI(tag, "- SSL supported");
	} else {
		ESP_LOGI(tag, "- SSL NOT supported");
	}
	if (data->features & CURL_VERSION_LIBZ) {
		ESP_LOGI(tag, "- LIBZ supported");
	} else {
		ESP_LOGI(tag, "- LIBZ NOT supported");
	}
	if (data->features & CURL_VERSION_NTLM) {
		ESP_LOGI(tag, "- NTLM supported");
	} else {
		ESP_LOGI(tag, "- NTLM NOT supported");
	}
	if (data->features & CURL_VERSION_DEBUG) {
		ESP_LOGI(tag, "- DEBUG supported");
	} else {
		ESP_LOGI(tag, "- DEBUG NOT supported");
	}
	if (data->features & CURL_VERSION_UNIX_SOCKETS) {
		ESP_LOGI(tag, "- UNIX sockets supported");
	} else {
		ESP_LOGI(tag, "- UNIX sockets NOT supported");
	}
  ESP_LOGI(tag, "Protocols:");
	int i=0;
	while(data->protocols[i] != NULL) {
		ESP_LOGI(tag, "- %s", data->protocols[i]);
		i++;
	}
}

HttpRequest::~HttpRequest() {
  free(_url);
  curl_easy_cleanup(handle);
  globalInit--;
  if(globalInit == 0) {
    curl_global_cleanup();
  }
}

uint8_t HttpRequest::get() {
  curl_easy_setopt(handle, CURLOPT_URL, _url);
  curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&match);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeData);

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

uint8_t HttpRequest::getHttps() {
  curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
  //curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1L);
  //curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(handle, CURLOPT_URL, _url);
  curl_easy_setopt(handle, CURLOPT_CAINFO, NULL);
  curl_easy_setopt(handle, CURLOPT_CAPATH, NULL);
  curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 2L);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&match);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, *writeData);
  //curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, *writeData);
  curl_easy_setopt(handle, CURLOPT_POSTFIELDS, "postFields");

  ESP_LOGD(tag, "curl_easy_perform()...");

  CURLcode res = curl_easy_perform(handle);

  if(res != CURLE_OK) {
    ESP_LOGD(tag, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));
  } else {
    ESP_LOGD(tag, "curl_easy_perform() succeeded.");
  }

  /* always cleanup */
  curl_easy_cleanup(handle);
  return 0;
}

void HttpRequest::setPostFields(const char* postFields) {
  curl_easy_setopt(handle, CURLOPT_POSTFIELDS, postFields);
}

size_t HttpRequest::writeData(void *buffer, size_t size, size_t nmemb, void *userp) {
	ESP_LOGD(tag, "writeData called: buffer=0x%lx, size=%d, nmemb=%d",
      (unsigned long)buffer, size, nmemb);
	ESP_LOGI(tag, "data>> %.*s", size*nmemb, (char *)buffer);

  struct Match* _match = (struct Match*)userp;
  if(_match != nullptr && _match->string != nullptr) {
    _match->result = (strncmp(_match->string, (char*)buffer, (size * nmemb) - 1) == 0);
  }
	return size * nmemb;
}

} // namespace mrdunk
