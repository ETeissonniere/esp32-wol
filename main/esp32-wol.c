#include "esp_event.h"
#include "esp_netif.h"

#include "ethernet_app.h"
#include "wifi_app.h"

void app_main(void) {
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(ethernet_app_start());
  ESP_ERROR_CHECK(wifi_app_start());
}
