#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

/**
 * @brief Register the HTTP handler responsible for triggering Wake-on-LAN.
 *
 * @param server Handle to the HTTP server instance.
 * @return ESP_OK on success, error code from ESP-IDF otherwise.
 */
esp_err_t http_api_wol_register(httpd_handle_t server);
