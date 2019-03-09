#include <stdio.h>
#include <assert.h>

#include "../mqttHelpers.cpp"

/* Unit tests for mqtt.h matchTopic() method.
 * `g++ tests/mqtt.cpp` will compile these tests. */
int main() {
  printf("~~~~~~~~~~~~~~~~~\n");
  const char testTopic[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/00000536b89a";
  {
    const char match[] = "7abr";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "7abr/";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "#";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/#";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/sensor/#";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/#";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/WRONG/#";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/00000536b89a/#";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/WRONG";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/00000536b89a/WRONG";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/00000536b89a";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "+/sensor/temperature/inside_downstairs/raspbmc/00000536b89a";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/+/temperature/inside_downstairs/raspbmc/00000536b89a";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/+";
    printf("Match   \t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 1);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/raspbmc/+/WRONG";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "+/temperature/inside_downstairs/raspbmc/00000536b89a";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  {
    const char match[] = "7abr/sensor/temperature/inside_downstairs/+";
    printf("No Match\t%s %s\n", testTopic, match);
    assert(matchTopic(match, testTopic) == 0);
  }
  printf("~~~~~~~~~~~~~~~~~\n");

  return 0;
}

