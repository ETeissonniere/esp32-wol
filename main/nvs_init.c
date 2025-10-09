#include "nvs_init.h"

#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "common.h"

esp_err_t nvs_init_or_reset(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGI(TAG, "Erasing NVS");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  return err;
}