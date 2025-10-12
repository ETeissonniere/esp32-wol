#include "http_api_index.h"

#include <stdint.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"

#include "common.h"
#include "http_api_shared.h"

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

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

esp_err_t http_api_index_register(httpd_handle_t server) {
  const esp_err_t err = httpd_register_uri_handler(server, &get_root);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "failed to register HTTP handler (%s): %s", get_root.uri,
             esp_err_to_name(err));
  }
  return err;
}
