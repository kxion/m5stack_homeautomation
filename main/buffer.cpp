#include <M5Stack.h>
#include <string.h>
#include <stdio.h>
#include <freertos/semphr.h>
extern "C" {
#include "flatmap.h"
}
#include "mqtt.h"

static SemaphoreHandle_t mutex;

static MapInfo mapInfo;
static uint8_t initialized = 0;

static void assignContentChild(const void* source, void** destination) {
  free(*destination);
  *destination = (void*)strdup((const char*)source);
}

static void freeContentChild(void* content) {
  free(content);
  content = nullptr;
}

static void assignContent(const void* source, void** destination) {
  if(*destination == nullptr) {
    // Nothing currently assigned to Entry.
    *destination = malloc(sizeof(MapInfo));
    *((MapInfo*)(*destination)) = (MapInfo) {
      .entries = nullptr,
      .entriesMax = 16,
      .entriesLen = 0,
      .freeContent = freeContentChild,
      .assignContent = assignContentChild
    };
    flatmap_init((MapInfo*)(*destination));
  }

  char sourceCp[strlen((char*)source) + 1];
  strcpy(sourceCp, (char*)source);
  char* sourceCp_p = sourceCp;
  char* key = nullptr;
  char* value = nullptr;
  while(1) {
    key = strsep(&sourceCp_p, ":");
    if(key && strlen(key) > 0) {
      value = strsep(&sourceCp_p, ",\0");
    }
    
    if((key == nullptr || strlen(key) == 0) && (value == nullptr || strlen(value) == 0)) {
      break;
    } else if(key != nullptr && strlen(key) > 0 && (value == nullptr || strlen(value) == 0)) {
      Entry newEntry = {
        .key = (char*)"no_key",
        .content = key
      };
      flatmap_put((MapInfo*)(*destination), &newEntry);
    } else if((key == nullptr || strlen(key) == 0) && value != nullptr && strlen(value) > 0) {
      Entry newEntry = {
        .key = (char*)"no_key",
        .content = value
      };
      flatmap_put((MapInfo*)(*destination), &newEntry);
    } else {
      Entry newEntry = {
        .key = key,
        .content = value
      };
      flatmap_put((MapInfo*)(*destination), &newEntry);
    }
    key = nullptr;
    value = nullptr;
  }
  char buffer[50];
  snprintf(buffer, 50, "%lu", millis());
  Entry newEntry = {
    .key = (char*)"timestamp",
    .content = buffer
  };
  flatmap_put((MapInfo*)(*destination), &newEntry);
}

static void freeContent(void* content) {
  flatmap_free((MapInfo*)content);
  content = nullptr;
  free(content);
  content = nullptr;
}

static void initialize() {
  if(initialized) {
    return;
  }
  initialized = 1;
  mapInfo = (MapInfo) {
    .entries = nullptr,
    .entriesMax = 16,
    .entriesLen = 0,
    .freeContent = freeContent,
    .assignContent = assignContent
  };
  flatmap_init(&mapInfo);

  mutex = xSemaphoreCreateMutex();
  assert(mutex && "Unable to create mutex");
}

void mqttBuffer_dispalyAll(MapInfo* _mapInfo) {
  initialize();

  xSemaphoreTake(mutex, portMAX_DELAY);
  printf("-----------------\n");
  for(size_t index = 0; index < flatmap_getEntryCount(_mapInfo); index++) {
    Entry* entry = flatmap_getByIndex(_mapInfo, index);
    printf("%s\t:\n", entry->key);
    for(size_t indexInner = 0;
        indexInner < flatmap_getEntryCount((MapInfo*)entry->content);
        indexInner++)
    {
      Entry* entryInner = flatmap_getByIndex((MapInfo*)entry->content, indexInner);
      printf("\t %-20s: %s\n", entryInner->key, (char*)entryInner->content);
    }
  }
  xSemaphoreGive(mutex);
}

void mqttBuffer_dispalyAll() {
  mqttBuffer_dispalyAll(&mapInfo);
}

static void assignContentCopy(const void* source, void** destination) {
  assert(*destination == nullptr && "Entry already exists.");

  // Nothing currently assigned to Entry.
  *destination = malloc(sizeof(MapInfo));
  *((MapInfo*)(*destination)) = (MapInfo) {
    .entries = nullptr,
      .entriesMax = 16,
      .entriesLen = 0,
      .freeContent = freeContentChild,
      .assignContent = assignContentChild
  };
  flatmap_init((MapInfo*)(*destination));

  for(size_t indexInner = 0;
      indexInner < flatmap_getEntryCount((MapInfo*)source);
      indexInner++)
  {
    Entry* entryInner = flatmap_getByIndex((MapInfo*)source, indexInner);
    flatmap_put((MapInfo*)(*destination), entryInner);
  }
}

static void freeContentCopy(void* content) {
  flatmap_free((MapInfo*)content);
  assert(((MapInfo*)content)->entriesLen == 0);
  assert(((MapInfo*)content)->entries == nullptr);
  free(content);
  content = nullptr;
}

void mqttBuffer_getMatching(const char* match, MapInfo* mapCopy) {
  initialize();

  xSemaphoreTake(mutex, portMAX_DELAY);

  *mapCopy = (MapInfo) {
    .entries = nullptr,
    .entriesMax = 16,
    .entriesLen = 0,
    .freeContent = freeContentCopy,
    .assignContent = assignContentCopy
  };
  flatmap_init(mapCopy);

  for(size_t index = 0; index < flatmap_getEntryCount(&mapInfo); index++) {
    Entry* entry = flatmap_getByIndex(&mapInfo, index);
    if(matchTopic(match, entry->key) > 0) {
      flatmap_put(mapCopy, entry);
    }
  }

  xSemaphoreGive(mutex);
}

void mqttBuffer_put(const char* topic, const int topicLen, const char* data, const int dataLen) {
  initialize();

  char topicCopy[topicLen + 1];
  strncpy(topicCopy, topic, topicLen);
  topicCopy[topicLen] = '\0';

  char dataCopy[dataLen + 1];
  strncpy(dataCopy, data, dataLen);
  dataCopy[dataLen] = '\0';

  Entry newData = {
    .key = topicCopy,
    .content = dataCopy
  };
  xSemaphoreTake(mutex, portMAX_DELAY);
  flatmap_put(&mapInfo, &newData);
  xSemaphoreGive(mutex);

  mqttBuffer_dispalyAll();
}
