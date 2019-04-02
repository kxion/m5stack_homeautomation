#include "Screen.h"
#include <ButtonInfoBar.h>
#include <StatusBar.h>
#include <Button.h>
#include <WidgetMosaic.h>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <AppScreen.h>
#include <UpDownButton.h>
#include <WiFi.h>

#include <limits.h>

#include "mqtt.h"
#include "config.h"
#include "buffer.h"
extern "C" {
#include "flatmap.h"
}

using namespace Codingfield::UI;

#define HEATING_CONTROL GLOBAL_ID "/heating/switch/override"

// Forward definition.
void updateButton1();
static void updateButton1(const int32_t value);
static void publishButton1(const int32_t value);

static AppScreen* screen;
static StatusBar* topBar;
static ButtonInfoBar* bottomBar;
static Codingfield::UI::Button* button0;
static UpDownButton* button1;
static Codingfield::UI::Button* button2;
static WidgetMosaic* mosaic;
static Widget* focus;

static int32_t editButton1Value = 0;
static int32_t editOldButton1Value = 0;
static boolean editingButton1Value = false;

static int32_t uptimeHours = 0;
static bool longPush = false;
static std::vector<StatusBar::WifiStatuses> wifiStatus {
    StatusBar::WifiStatuses::No_signal,
    StatusBar::WifiStatuses::Weak,
    StatusBar::WifiStatuses::Medium,
    StatusBar::WifiStatuses::Full
};

static uint8_t mqttSubscriptions = 0;

void setupMenu() {
  // Instantiate and configure all widgets
  topBar = new StatusBar();
  bottomBar = new ButtonInfoBar();
  mosaic = new WidgetMosaic(1, 3);
  screen = new AppScreen(Size(320, 240), BLACK, topBar, bottomBar, mosaic);

  // Give the focus to the main screen
  focus = screen;

  button0 = new Codingfield::UI::Button(mosaic);
  button0->SetBackgroundColor(LIGHTGREY);
  button0->SetTextColor(WHITE);
  button0->SetText("19C");
  button0->SetTitle("Temperature");
  button0->SetSelected(true);
  button0->SetHighlightColor(WHITE);

  button1 = new UpDownButton(mosaic); // Up/Down button
  button1->SetBackgroundColor(LIGHTGREY);
  button1->SetTextColor(BLACK);
  button1->SetText("0");
  button1->SetTitle("auto");
  button1->SetHighlightColor(WHITE);

  button2 = new Codingfield::UI::Button(mosaic);
  button2->SetBackgroundColor(ORANGE);
  button2->SetTextColor(BLACK);
  button2->SetText("50%");
  button2->SetHighlightColor(WHITE);

  topBar->SetUptime(0);
  topBar->SetWifiStatus(StatusBar::WifiStatuses::No_signal);

  bottomBar->SetButtonAText("<");
  bottomBar->SetButtonBText("SELECT");
  bottomBar->SetButtonCText(">");

  // Callback called by the mosaic when it changes mode (mosaic/zoom on 1 widget)
  // We use it to update the bottom bar.
  mosaic->SetZoomOnSelectedCallback([](Widget* widget, bool edit) {
    if(edit) {
      if(widget->IsEditable()){
        bottomBar->SetButtonAText("-");
        bottomBar->SetButtonBText("APPLY");
        bottomBar->SetButtonCText("+");
      } else {
        bottomBar->SetButtonAText("");
        bottomBar->SetButtonBText("BACK");
        bottomBar->SetButtonCText("");
      }
    } else {
      bottomBar->SetButtonAText("<");
      bottomBar->SetButtonBText("SELECT");
      bottomBar->SetButtonCText(">");
    }
  });

  // Configure callback to be called when the user wants to increment the value
  // of button1
  button1->SetUpCallback([](UpDownButton* w) {
    editingButton1Value = true;
    if(editButton1Value < 0) {
      editButton1Value = 0;
    } else if(editButton1Value < 120 * 60 * 1000) {
      editButton1Value += 30 * 60 * 1000;
    }
    updateButton1(editButton1Value);
    return true;
  });

  // Configure callback to be called when the user wants to decrement the value
  // of button1
  button1->SetDownCallback([](UpDownButton* w) {
    editingButton1Value = true;
    if(editButton1Value > 0) {
      editButton1Value = 0;
    } else if(editButton1Value == 0) {
      editButton1Value -= 30 * 60 * 1000;
    } else if(editButton1Value >= INT32_MIN / 2) {
      editButton1Value *= 2;
    }
    updateButton1(editButton1Value);
    return true;
  });

  // Configure callback to be called when the user wants to apply the value
  // of button1
  button1->SetApplyCallback([](UpDownButton* w) {
    editingButton1Value = false;
    editOldButton1Value = editButton1Value;
    publishButton1(editOldButton1Value);
    return false;
  });

  // Configure callback to be called when the user wants to cancel modification
  // of the value of button1
  button1->SetCancelCallback([](UpDownButton* w) {
    editingButton1Value = false;
    editButton1Value = editOldButton1Value;
    updateButton1();
    return true;
  });

  // Draw the screen and all its children
  screen->Draw();
}

void updateMenu() {
  if(!mqttSubscriptions) {
      // Subscribe to the heating "override" topic so we get updated is any other
      // controller modifies the value.
      mqttSubscriptions = mqtt_subscribe(HEATING_CONTROL);
  }

  int8_t rssi = WiFi.RSSI();
  if(rssi >= -55) {
    topBar->SetWifiStatus(StatusBar::WifiStatuses::Full);
  } else if(rssi >= -75) {
    topBar->SetWifiStatus(StatusBar::WifiStatuses::Medium);
  } else if(rssi >= -85) {
    topBar->SetWifiStatus(StatusBar::WifiStatuses::Weak);
  } else {
    topBar->SetWifiStatus(StatusBar::WifiStatuses::No_signal);
  }

  // Notify the widgets that physical buttons are pressed
  if(M5.BtnA.wasPressed()) {
    focus->OnButtonAPressed();
  }

  if(M5.BtnB.pressedFor(1000)) {
    if(!longPush) {
      focus->OnButtonBLongPush();
      longPush = true;
    }
  }
  else if(M5.BtnB.wasReleased()) {
    if(!longPush) {
      focus->OnButtonBPressed();
    }
    else {
      longPush = false;
    }
  }

  if(M5.BtnC.wasPressed()) {
    focus->OnButtonCPressed();
  }

  uptimeHours = millis() / (60*60000);
  topBar->SetUptime(uptimeHours);

  /*char strftime_buf[64];
  snprintf(strftime_buf, 64, "%02d:%02d:%02d", 12, 14, 59);
  topBar->SetDateTime(strftime_buf);*/

  // Redraw the screen
  screen->Draw();
}

void setButton0(const char* text, const char* title) {
  if(text != nullptr) {
    button0->SetText(text);
  }
  if(title != nullptr) {
    button0->SetTitle(title);
  }
}

void setButton1(const uint8_t state) {
  if(state > 0) {
    button1->SetText("Heating on");
  } else {
    button1->SetText("Heating off");
  }
  updateButton1();
}

static String duration(const int32_t _value) {
  const int32_t value = ((_value > 0) ? (_value / 1000) : (-_value / 1000));
  if(value < 60) {
    return String(value) + String("sec");
  }
  if(value < 60 * 60) {
    const int32_t mins = value / 60;
    const int32_t secs = value % 60;
    char buffer[10] = "";
    snprintf(buffer, 10, "%d:%02dmin", mins, secs);
    return String(buffer);
  }
  if(value < 24 * 60 * 60) {
    const int32_t hours = value / (60 * 60);
    const int32_t mins = (value % (60 * 60)) / 60;
    const int32_t secs = value % 60;
    char buffer[16] = "";
    snprintf(buffer, 10, "%d:%02d:%02dhour", hours, mins, secs);
    return String(buffer);
  }
  if(value < 7 * 24 * 60 * 60) {
    return String(value / (24 * 60 * 60)) + String("day");
  }
  if(value < 4 * 7 * 24 * 60 * 60) {
    return String(value / (7 * 24 * 60 * 60)) + String("week");
  }
  return String(value / (4 * 7 * 24 * 60 * 60)) + String("month");
}

void updateButton1() {
    if(editingButton1Value) {
      // Don't update the values while we are editing them.
      return;
    }
    // Get a snapshot of the topics we care about.
    MapInfo mapCopy;
    mqttBuffer_getMatching(HEATING_CONTROL, &mapCopy);

    size_t buttonCount = flatmap_getEntryCount(&mapCopy);
    if(buttonCount == 0) {
      // Nothing in cache.
      flatmap_free(&mapCopy);
      return;
    }
    if(buttonCount > 1) {
      ESP_LOGW("menu", "More than one topic matching %s in cache.\n", HEATING_CONTROL);
    }

    // Presume the first returned entry is the one we want.
    Entry* entry = flatmap_getByIndex(&mapCopy, 0);
    Entry subEntry = {
      .key = (char*)"forMs",
      .content = nullptr
    };
    Entry* result = flatmap_get((MapInfo*)entry->content, &subEntry);
    uint32_t forMs = strtod((char*)result->content, NULL);

    Entry subEntry2 = {
      .key = (char*)"timestamp",
      .content = nullptr
    };
    Entry* result2 = flatmap_get((MapInfo*)entry->content, &subEntry2);
    uint32_t timestamp = strtod((char*)result2->content, NULL);

    // Calculate remaining time.
    uint32_t timeRemaining = forMs - (millis() - timestamp);
    updateButton1(timeRemaining);

    flatmap_free(&mapCopy);
}

static void updateButton1(const int32_t value) {
  printf("updateButton1(%i)\n", value);
  String message;
  if(value < 0) {
    message = String("Force off for ") + duration(value);
  } else if(value > 0) {
    message = String("Force on for ") + duration(value);
  } else {
    message = "auto";
  }
  button1->SetTitle(message.c_str());
}

static void publishButton1(const int32_t value) {
  String payload("state:");
  payload += (value > 0) ? String("on") : String("off");
  payload += String(",forMs:");
  payload += (value > 0) ? String(value) : String(-value);
  mqtt_publish(HEATING_CONTROL, payload.c_str());
}

