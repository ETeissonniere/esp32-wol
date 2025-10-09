#include "ethernet_iface.h"

#include "esp_err.h"
#include "esp_eth_netif_glue.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "ethernet_init.h"

#include "common.h"

static esp_eth_handle_t s_eth_handle;

static esp_err_t ethernet_iface_init_handle(void) {
  uint8_t eth_port_cnt = 0;
  esp_eth_handle_t *eth_handles = NULL;
  esp_err_t err = ethernet_init_all(&eth_handles, &eth_port_cnt);
  if (err != ESP_OK) {
    return err;
  }
  if (eth_port_cnt != 1 || eth_handles == NULL) {
    ESP_LOGE(TAG, "Unexpected Ethernet port count: %u", (unsigned)eth_port_cnt);
    return ESP_ERR_INVALID_STATE;
  }

  s_eth_handle = eth_handles[0];
  return ESP_OK;
}

static esp_err_t ethernet_iface_attach_netif(void) {
  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
  esp_netif_t *eth_netif = esp_netif_new(&cfg);
  if (eth_netif == NULL) {
    return ESP_ERR_NO_MEM;
  }
  return esp_netif_attach(eth_netif, esp_eth_new_netif_glue(s_eth_handle));
}

static esp_err_t ethernet_iface_start_driver(void) {
  esp_err_t err = ethernet_iface_attach_netif();
  if (err != ESP_OK) {
    return err;
  }
  return esp_eth_start(s_eth_handle);
}

static void ethernet_iface_log_device_info(void) {
  eth_dev_info_t info = ethernet_init_get_dev_info(s_eth_handle);
  ESP_LOGI(TAG, "Device Name: %s", info.name);
  ESP_LOGI(TAG, "Device type: %d", info.type);
  ESP_LOGI(TAG, "Pins: cs: %d, intr: %d", info.pin.eth_spi_cs,
           info.pin.eth_spi_int);
}

esp_err_t ethernet_iface_start(void) {
  ESP_ERROR_CHECK(ethernet_iface_init_handle());
  ESP_ERROR_CHECK(ethernet_iface_start_driver());
  ethernet_iface_log_device_info();
  return ESP_OK;
}
