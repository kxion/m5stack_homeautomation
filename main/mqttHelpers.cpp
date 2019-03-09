#include "mqtt.h"

uint8_t matchTopic(const char* match, const char* topic) {
  char matchCpy[strlen(match) + 1];
  strcpy(matchCpy, match);
  char* matchCpy_p = matchCpy;
  char topicCpy[strlen(topic) + 1];
  strcpy(topicCpy, topic);
  char* topicCpy_p = topicCpy;
  char* tokenMatch = nullptr;
  char* tokenTopic = nullptr;
  while(1) {
    tokenMatch = strsep(&matchCpy_p, "/\n");
    tokenTopic = strsep(&topicCpy_p, "/\n");
    if(tokenTopic == nullptr && tokenMatch == nullptr) {
      return 1;
    }
    if(tokenTopic != nullptr && tokenMatch == nullptr) {
      return 0;
    }
    if(tokenTopic == nullptr && tokenMatch != nullptr) {
      return 0;
    }
    if(strcmp(tokenMatch, "#") == 0) {
      return 1;
    }
    if(strcmp(tokenMatch, tokenTopic) != 0 && strcmp(tokenMatch, "+") != 0) {
      return 0;
    }
  }

  return 1;
}

