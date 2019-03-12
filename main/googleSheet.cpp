#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "googleSheet.h"

static const char *TAG = "googleSheet";

static const char *REQUEST = "GET %s HTTP/1.0\r\n"
    "Host: " WEB_SERVER "\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

/* Root cert for google.com, taken from server_root_cert.pem

   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.google.com:443 </dev/null

   The CA root cert is the last cert given in the chain of certs.

   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

uint8_t googleSheet_connect(const char* url) {
  char buf[512];
  int ret, len;

  esp_tls_cfg_t cfg = {};
  cfg.cacert_pem_buf = server_root_cert_pem_start;
  cfg.cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start;
  printf("%p\t%p\n", server_root_cert_pem_start, server_root_cert_pem_end);

  ESP_LOGI(TAG, "Connect to %s", url);
  struct esp_tls *tls = esp_tls_conn_http_new(url, &cfg);

  if(tls != NULL) {
    ESP_LOGI(TAG, "Connection established...");
  } else {
    ESP_LOGE(TAG, "Connection failed...");
    esp_tls_conn_delete(tls);    
    return 0;
  }
  
  size_t written_bytes = 0;
  char request[strlen(url) + strlen(REQUEST)] = "";
  do {
    snprintf(request, strlen(url) + strlen(REQUEST), REQUEST, url);
    ret = esp_tls_conn_write(tls, 
                             request + written_bytes, 
                             strlen(request) - written_bytes);
    if (ret >= 0) {
      ESP_LOGI(TAG, "%d bytes written", ret);
      written_bytes += ret;
    } else if (ret != MBEDTLS_ERR_SSL_WANT_READ  && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
      esp_tls_conn_delete(tls);    
      return 0;
    }
  } while(written_bytes < strlen(request));

  ESP_LOGI(TAG, "Reading HTTP response...");

  do
  {
    len = sizeof(buf) - 1;
    bzero(buf, sizeof(buf));
    ret = esp_tls_conn_read(tls, (char *)buf, len);

    if(ret == MBEDTLS_ERR_SSL_WANT_WRITE  || ret == MBEDTLS_ERR_SSL_WANT_READ)
      continue;

    if(ret < 0)
    {
      ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
      break;
    }

    if(ret == 0)
    {
      ESP_LOGI(TAG, "connection closed");
      break;
    }

    len = ret;
    ESP_LOGD(TAG, "%d bytes read", len);
    /* Print response directly to stdout as it is read */
    for(int i = 0; i < len; i++) {
      putchar(buf[i]);
    }
  } while(1);

  esp_tls_conn_delete(tls);
  return 1;
}

