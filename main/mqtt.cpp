#include <vector>

#include "esp_log.h"
#include "mqtt_client.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "freertos/event_groups.h"

#include "mqtt.h"
#include "buffer.h"
#include "config.h"


#define BROKER_CONNECT_TOPIC GLOBAL_ID "/broker/connect/" DEVICE_NAME

static const char *TAG = "MQTT_EXAMPLE";

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;
static esp_mqtt_client_handle_t client;
static std::vector<char*> subscriptions;

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
    //esp_mqtt_client_handle_t client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_publish(BROKER_CONNECT_TOPIC, "state:true");
            for(std::vector<char*>::iterator it = subscriptions.begin();
                it != subscriptions.end();
                ++it) {
              esp_mqtt_client_subscribe(client, *it, 0);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_publish(BROKER_CONNECT_TOPIC, "state:false");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);

            mqttBuffer_put(event->topic, event->topic_len, event->data, event->data_len);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_init(void) {
  nvs_flash_init();
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  wifi_config_t wifi_config = {
    .sta = {
      {.ssid = CONFIG_WIFI_SSID},
      {.password = CONFIG_WIFI_PASSWORD},
      .scan_method = WIFI_ALL_CHANNEL_SCAN,
    },
  };
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_LOGI(TAG, "start the WIFI SSID:[%s]", CONFIG_WIFI_SSID);
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGI(TAG, "Waiting for wifi");
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void mqtt_app_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        //uri: CONFIG_BROKER_URL,
        //port: nullptr,
        //client_id: nullptr,
        //event_handle: mqtt_event_handler,
        // .user_context = (void *)your_context
    };
    mqtt_cfg.uri = CONFIG_BROKER_URL;
    mqtt_cfg.event_handle = mqtt_event_handler;

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void mqtt_publish(const char* topic, const char* message) {
  esp_mqtt_client_publish(client, topic, message, 0, 0, 0);
  mqttBuffer_put(topic, strlen(topic), message, strlen(message));
}

void mqtt_subscribe(const char* topic) {
  // Save subscribed topics incase we need to re-connect to broker.
  char* persistentTopic = strdup(topic);
  subscriptions.push_back(persistentTopic);
  esp_mqtt_client_subscribe(client, topic, 0);
}

