#pragma once

#include "esp_err.h"

/**
 * @brief Initialise, attach and start the Ethernet interface.
 *
 * The caller must ensure that `esp_netif_init()` and `esp_event_loop_create_default()`
 * have been invoked before calling this function. The implementation assumes there is
 * exactly one Ethernet port configured via `ethernet_init_all`.
 */
esp_err_t ethernet_app_start(void);
