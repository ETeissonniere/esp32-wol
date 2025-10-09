#pragma once

#include "esp_err.h"

/**
 * @brief Initialise, and reset if necessary the NVS flash.
 */
esp_err_t nvs_init_or_reset(void);
