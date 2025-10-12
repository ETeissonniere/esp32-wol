#include "http_api_wol.h"

#include <string.h>

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"

#include "common.h"
#include "ethernet_iface.h"
#include "ethernet_wol.h"
#include "http_api_shared.h"

#define HTTPD_RESPONSE_NO_LINK "Ethernet link is down"
#define HTTPD_RESPONSE_WOL_FAILED                                              \
  "Ethernet link is up, but we were unable to send the WoL packet"
#define HTTPD_RESPONSE_WOL_SUCCESS                                             \
  "WoL packet successfully sent, server should wake up soon"

static esp_err_t http_api_handler_post_wol(httpd_req_t *req) {
  if (!ethernet_iface_link_is_up()) {
    httpd_resp_set_status(req, HTTPD_STATUS_503);
    httpd_resp_send(req, HTTPD_RESPONSE_NO_LINK,
                    strlen(HTTPD_RESPONSE_NO_LINK));
  } else {
    esp_err_t err = ethernet_wol_send();
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "sending WoL packet failed");
      httpd_resp_set_status(req, HTTPD_STATUS_500);
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

esp_err_t http_api_wol_register(httpd_handle_t server) {
  const esp_err_t err = httpd_register_uri_handler(server, &post_wol);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "failed to register HTTP handler (%s): %s", post_wol.uri,
             esp_err_to_name(err));
  }
  return err;
}
