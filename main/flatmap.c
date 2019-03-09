#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "flatmap.h"

void trimWhitespace(char* string) {
  uint8_t leadingSpace = 1;
  size_t i = 0;
  size_t j = 0;

  // Remove leading spaces first.
  size_t len = strlen(string);
  for(; i < len; i++) {
    int c = string[i];
    if(isspace(c) == 0 || leadingSpace == 0) {
      leadingSpace = 0;
      string[j] = string[i];
      j++;
    }
  }
  string[j] = '\0';

  // Remove trailing whitespace.
  j--;
  while(isspace((int)string[j]) != 0) {
    string[j] = '\0';
    j--;
  }
}

void flatmap_init(MapInfo* mapInfo) {
  assert(mapInfo->entries == NULL && "MapInfo not empty.");
  if(mapInfo->entriesMax == 0) {
    mapInfo->entriesMax = 16;
  }
  mapInfo->entriesLen = 0;
  mapInfo->entries = calloc(mapInfo->entriesMax, sizeof(Entry));
  assert(mapInfo->entries != NULL && "Error: malloc fail");
}

void flatmap_free(MapInfo* mapInfo) {
  while(mapInfo->entriesLen > 0) {
    Entry* entry = flatmap_getByIndex(mapInfo, mapInfo->entriesLen - 1);
    free(entry->key);
    entry->key = NULL;
    mapInfo->freeContent(entry->content);
    entry->content = NULL;
    mapInfo->entriesLen--;
  }
  free(mapInfo->entries);
  mapInfo->entries = NULL;
}

Entry* flatmap_getByIndex(MapInfo* mapInfo, const size_t index) {
  if(index >= mapInfo->entriesLen) {
    return NULL;
  }
  return &(mapInfo->entries[index]);
}

size_t flatmap_getEntryCount(MapInfo* mapInfo) {
  return mapInfo->entriesLen;
}

Entry* flatmap_get(MapInfo* mapInfo, const Entry* getEntry) {
  for(size_t pos = 0; pos < mapInfo->entriesLen; pos++) {
    char* tmpKey = strdup(getEntry->key);
    assert(tmpKey != NULL && "Error: strdup fail");
    trimWhitespace(tmpKey);
    if(strcmp(mapInfo->entries[pos].key, tmpKey) == 0) {
      free(tmpKey);
      return &(mapInfo->entries[pos]);
    }
    free(tmpKey);
  }
  return NULL;
}

void flatmap_put(MapInfo* mapInfo, const Entry* newEntry) {
  Entry* candidate = flatmap_get(mapInfo, newEntry);
  if(candidate == NULL) {
    // Create new Entry by assigning an unused one from mapInfo->entries.
    if(mapInfo->entriesMax <= mapInfo->entriesLen) {
      // Need more space for new Entries.
      mapInfo->entriesMax *= 2;
      mapInfo->entries = realloc(mapInfo->entries, mapInfo->entriesMax);
      assert(mapInfo->entries != NULL && "Error: realloc fail");
    }

    candidate = &(mapInfo->entries[mapInfo->entriesLen]);
    mapInfo->entriesLen++;
    
    candidate->content = NULL;

    // Make persistent copy of key with strdup().
    //candidate->key = strdup(newEntry->key);
    char* tmp = strdup(newEntry->key);
    assert(tmp != NULL && "Error: strdup fail");
    trimWhitespace(tmp);
    candidate->key = strdup(tmp);
    assert(candidate->key != NULL && "Error: strdup fail");
    free(tmp);
  }
  // Callback to assign new content.
  mapInfo->assignContent(newEntry->content, &(candidate->content));
}
