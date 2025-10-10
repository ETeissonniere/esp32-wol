#pragma once

#include "esp_err.h"

/**
 * @brief Send WoL packet on Ethernet iface for configured MAC address.
 */
esp_err_t ethernet_wol_send(void);