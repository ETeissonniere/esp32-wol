#pragma once

#include "esp_err.h"
#include "esp_eth_driver.h"

/**
 * @brief Initialise, attach and start the Ethernet interface.
 *
 * The caller must ensure that `esp_netif_init()` and
 * `esp_event_loop_create_default()` have been invoked before calling this
 * function. The implementation assumes there is exactly one Ethernet port
 * configured via `ethernet_init_all`.
 */
esp_err_t ethernet_iface_start(void);

/**
 * @brief Returns whether the Ethernet link is up or down.
 */
bool ethernet_iface_link_is_up(void);

/**
 * @brief Returns the Ethernet handle used to send Ethernet packets.
 */
esp_eth_handle_t ethernet_iface_get_handle(void);