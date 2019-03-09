#ifndef DUNK__ESP32__FLATMAP_H
#define DUNK__ESP32__FLATMAP_H

#include <stdint.h>
#include <stdlib.h>

/* An entry in the FlatMap. */
struct Entry {
  char* key;
  void* content;
};
typedef struct Entry Entry;

/* Information about the FlatMap collection. */
struct MapInfo {
  Entry* entries;
  size_t entriesMax;
  size_t entriesLen;
  void(*freeContent)(void* content);
  void(*assignContent)(const void* source, void** destination);
};
typedef struct MapInfo MapInfo;

/* Initialise a new FlatMap. */
void flatmap_init(MapInfo* mapInfo);

/* Free all memory used by FlatMap. */
void flatmap_free(MapInfo* mapInfo);

/* Get an entry from the FlatMap by index nunber. */
Entry* flatmap_getByIndex(MapInfo* mapInfo, const size_t index);

/* Get the total number of entries in the FlatMap. */
size_t flatmap_getEntryCount(MapInfo* mapInfo);

/* Get an entry from the FlatMap matching getEntry->key. */
Entry* flatmap_get(MapInfo* mapInfo, const Entry* getEntry);

/* Store an entry in the flatMap. */
void flatmap_put(MapInfo* mapInfo, const Entry* newEntry);


#endif  // DUNK__ESP32__FLATMAP_H
