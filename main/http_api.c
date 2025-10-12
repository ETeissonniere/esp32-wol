#include "http_api.h"

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif_types.h"
#include "esp_wifi_types_generic.h"

#include "common.h"
#include "http_api_index.h"
#include "http_api_wol.h"

static esp_err_t http_api_register_handlers(httpd_handle_t server) {
  esp_err_t err = http_api_index_register(server);
  if (err != ESP_OK) {
    return err;
  }

  err = http_api_wol_register(server);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}

static httpd_handle_t http_api_start_server(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  if (httpd_start(&server, &config) == ESP_OK) {
    if (http_api_register_handlers(server) != ESP_OK) {
      httpd_stop(server);
      return NULL;
    }

    ESP_LOGI(TAG, "HTTP API server ready on port: '%d'", config.server_port);

    return server;
  }

  ESP_LOGE(TAG, "error starting HTTP API server");
  return NULL;
}

static esp_err_t http_api_stop_server(httpd_handle_t server) {
  return httpd_stop(server);
}

static void http_api_disconnect_handler(void *arg, esp_event_base_t event_base,
                                        int32_t event_id, void *event_data) {
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server) {
    ESP_LOGW(TAG, "WiFi disconnected, stopping HTTP API server");
    if (http_api_stop_server(*server) == ESP_OK) {
      *server = NULL;
    } else {
      ESP_LOGE(TAG, "failed to stop HTTP API server");
    }
  }
}

static void http_api_connect_handler(void *arg, esp_event_base_t event_base,
                                     int32_t event_id, void *event_data) {
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server == NULL) {
    ESP_LOGW(TAG, "WiFi reconnected, restarting HTTP API server");
    *server = http_api_start_server();
  }
}

esp_err_t http_api_start(void) {
  static httpd_handle_t server = NULL;

  // We turn on and off the HTTP server in case of WiFi disconnects/reconnects
  ESP_ERROR_CHECK(esp_event_handler_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &http_api_connect_handler, &server));
  ESP_ERROR_CHECK(
      esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
                                 &http_api_disconnect_handler, &server));

  // We also ensure we start the webserver on the first run
  server = http_api_start_server();
  if (server == NULL) {
    ESP_LOGE(TAG, "failed to start HTTP API server");
    return ESP_FAIL;
  }

  return ESP_OK;
}
