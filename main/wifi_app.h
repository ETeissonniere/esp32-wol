#pragma once

#include "esp_err.h"

/**
 * @brief Initialise, attach and start the WiFi interface.
 *
 * The caller must ensure that `esp_netif_init()` and
 * `esp_event_loop_create_default()` have been invoked before calling this
 * function.
 */
esp_err_t wifi_app_start(void);
