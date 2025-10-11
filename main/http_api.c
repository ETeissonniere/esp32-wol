#include "http_api.h"

#include <stdint.h>
#include <string.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif_types.h"
#include "esp_wifi_types_generic.h"

#include "common.h"
#include "ethernet_iface.h"
#include "ethernet_wol.h"

#define HTTPD_STATUS_200 "200 OK"
#define HTTPD_STATUS_503 "503 Unavailable"
#define HTTPD_STATUS_504 "504 Internal Server Error"
#define HTTPD_STATUS_202 "202 Accepted"
#define HTTPD_RESPONSE_NO_LINK "Ethernet link is down"
#define HTTPD_RESPONSE_WOL_FAILED                                              \
  "Ethernet link is up, but we were unable to send the WoL packet"
#define HTTPD_RESPONSE_WOL_SUCCESS                                             \
  "WoL packet succesfully sent, server should wake up soon"

extern const uint8_t index_html_start[] asm("_binary_www_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_www_index_html_end");

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static esp_err_t http_api_handler_get_root(httpd_req_t *req) {
  const size_t index_html_len = index_html_end - index_html_start;
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  httpd_resp_set_status(req, HTTPD_STATUS_200);
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  httpd_resp_send(req, (const char *)index_html_start, index_html_len);
  return ESP_OK;
}

static const httpd_uri_t get_root = {.uri = "/",
                                     .method = HTTP_GET,
                                     .handler = http_api_handler_get_root,
                                     .user_ctx = NULL};

static esp_err_t http_api_handler_post_wol(httpd_req_t *req) {
  if (!ethernet_iface_link_is_up()) {
    httpd_resp_set_status(req, HTTPD_STATUS_503);
    httpd_resp_send(req, HTTPD_RESPONSE_NO_LINK,
                    strlen(HTTPD_RESPONSE_NO_LINK));
  } else {
    esp_err_t err = ethernet_wol_send();
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "sending WoL packet failed");
      httpd_resp_set_status(req, HTTPD_STATUS_504);
      httpd_resp_send(req, HTTPD_RESPONSE_WOL_FAILED,
                      strlen(HTTPD_RESPONSE_WOL_FAILED));
    } else {
      httpd_resp_set_status(req, HTTPD_STATUS_202);
      httpd_resp_send(req, HTTPD_RESPONSE_WOL_SUCCESS,
                      strlen(HTTPD_RESPONSE_WOL_SUCCESS));
    }
  }

  return ESP_OK;
}

static const httpd_uri_t post_wol = {.uri = "/wol",
                                     .method = HTTP_POST,
                                     .handler = http_api_handler_post_wol,
                                     .user_ctx = NULL};

static esp_err_t http_api_register_handlers(httpd_handle_t server) {
  const httpd_uri_t *handlers[] = {&get_root, &post_wol};

  for (size_t i = 0; i < ARRAY_SIZE(handlers); ++i) {
    esp_err_t err = httpd_register_uri_handler(server, handlers[i]);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "failed to register HTTP handler (%s): %s",
               handlers[i]->uri, esp_err_to_name(err));
      return err;
    }
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
