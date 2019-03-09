#include "../flatmap.c"

void freeContentImplementation(void* content) {
}

void assignContentImplementation(const void* source, void** destination) {
  *destination = (void*)source;
}

int main() {
  MapInfo mapInfo = {
    .entries = NULL,
    .entriesMax = 16,
    .entriesLen = 0,
    .assignContent = assignContentImplementation
  };

  flatmap_init(&mapInfo);
  Entry *results = NULL;
  Entry e1 = {"testA","test1"};

  assert(flatmap_get(&mapInfo, results) == NULL);
  flatmap_put(&mapInfo, &e1);
  results = flatmap_get(&mapInfo, &e1);
  assert(results != NULL);
  assert(strcmp(e1.content, results->content) == 0);

  Entry e2 = {"testA","test2"};
  results = flatmap_get(&mapInfo, &e2);
  assert(results != NULL);
  assert(strcmp(e1.content, results->content) == 0);

  flatmap_put(&mapInfo, &e2);
  results = flatmap_get(&mapInfo, &e2);
  assert(results != NULL);
  assert(strcmp(e2.content, results->content) == 0);

  Entry e3 = {"testB","test3"};
  flatmap_put(&mapInfo, &e3);
  results = flatmap_get(&mapInfo, &e2);
  assert(results != NULL);
  assert(strcmp(e2.content, results->content) == 0);

  results = flatmap_get(&mapInfo, &e3);
  assert(results != NULL);
  assert(strcmp(e3.content, results->content) == 0);
  
  printf("All passed.\n");
  return 0;
}
