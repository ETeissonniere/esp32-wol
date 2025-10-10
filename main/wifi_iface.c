#include "wifi_iface.h"

#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include "common.h"
#include "sdkconfig.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about
 * two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WIFI_MAX_RETRY 5

static int s_retry_num = 0;

static void wifi_iface_event_handler(void *arg, esp_event_base_t event_base,
                                     int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < WIFI_MAX_RETRY) {
      esp_wifi_connect();
      s_retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    ESP_LOGI(TAG, "connect to the AP fail");
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

static esp_err_t wifi_iface_create_event_group(void) {
  s_wifi_event_group = xEventGroupCreate();
  if (s_wifi_event_group == NULL) {
    return ESP_ERR_NO_MEM;
  }
  return ESP_OK;
}

static esp_err_t wifi_iface_init_wifi_stack(void) {
  esp_netif_t *netif = esp_netif_create_default_wifi_sta();
  if (netif == NULL) {
    return ESP_ERR_NO_MEM;
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  return esp_wifi_init(&cfg);
}

static esp_err_t wifi_iface_register_event_handlers(
    esp_event_handler_instance_t *instance_any_id,
    esp_event_handler_instance_t *instance_got_ip) {
  esp_err_t err = esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_iface_event_handler, NULL,
      instance_any_id);
  if (err != ESP_OK) {
    return err;
  }

  err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                            &wifi_iface_event_handler, NULL,
                                            instance_got_ip);
  if (err != ESP_OK) {
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                          *instance_any_id);
  }
  return err;
}

static esp_err_t wifi_iface_start_sta(void) {

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = CONFIG_ESP_WIFI_SSID,
              .password = CONFIG_ESP_WIFI_PASSWORD,
          },
  };
  esp_err_t err = esp_wifi_set_mode(WIFI_MODE_STA);
  if (err != ESP_OK) {
    return err;
  }
  err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  if (err != ESP_OK) {
    return err;
  }
  err = esp_wifi_start();
  if (err != ESP_OK) {
    return err;
  }

  ESP_LOGI(TAG, "wifi started");
  return ESP_OK;
}

static esp_err_t wifi_iface_wait_for_connection(void) {
  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) {
    ESP_LOGI(TAG, "connected to ap SSID: %s", CONFIG_ESP_WIFI_SSID);
    return ESP_OK;
  } else if (bits & WIFI_FAIL_BIT) {
    ESP_LOGI(TAG, "failed to connect to SSID: %s", CONFIG_ESP_WIFI_SSID);
    return ESP_FAIL;
  } else {
    ESP_LOGE(TAG, "UNEXPECTED EVENT");
    return ESP_FAIL;
  }
}

esp_err_t wifi_iface_start(void) {
  ESP_ERROR_CHECK(wifi_iface_create_event_group());
  ESP_ERROR_CHECK(wifi_iface_init_wifi_stack());

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(
      wifi_iface_register_event_handlers(&instance_any_id, &instance_got_ip));
  ESP_ERROR_CHECK(wifi_iface_start_sta());

  ESP_ERROR_CHECK(wifi_iface_wait_for_connection());

  return ESP_OK;
}
