#include "ethernet_wol.h"

#include <stdio.h>
#include <string.h>

#include "esp_err.h"
#include "esp_eth_driver.h"
#include "esp_log.h"

#include "common.h"
#include "ethernet_iface.h"
#include "sdkconfig.h"

#define WOL_MAC_LEN 6
#define WOL_SYNC_BYTES 6
#define WOL_MAC_REPEAT_COUNT 16
#define WOL_ETHERTYPE 0x0842
#define WOL_SYNC_BYTE 0xFF

// Convert MAC address from colon-separated string into byte array.
static esp_err_t ethernet_wol_parse_mac(const char *mac_str,
                                        uint8_t mac[WOL_MAC_LEN]) {
  int converted = sscanf(mac_str,
                         "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  if (converted != WOL_MAC_LEN) {
    ESP_LOGE(TAG, "Invalid WoL target MAC: %s", mac_str);
    return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}

esp_err_t ethernet_wol_send(void) {
  uint8_t target_mac[WOL_MAC_LEN];
  esp_err_t err = ethernet_wol_parse_mac(CONFIG_WOL_TARGET_MAC, target_mac);
  if (err != ESP_OK) {
    return err;
  }

  esp_eth_handle_t eth = ethernet_iface_get_handle();
  if (eth == NULL) {
    ESP_LOGE(TAG, "Ethernet handle not initialised");
    return ESP_ERR_INVALID_STATE;
  }

  // Build WoL payload: sync bytes followed by repeated target MAC.
  uint8_t payload[WOL_SYNC_BYTES + (WOL_MAC_REPEAT_COUNT * WOL_MAC_LEN)];
  memset(payload, WOL_SYNC_BYTE, WOL_SYNC_BYTES);

  size_t payload_len = WOL_SYNC_BYTES;
  for (size_t i = 0; i < WOL_MAC_REPEAT_COUNT; ++i) {
    memcpy(payload + payload_len, target_mac, WOL_MAC_LEN);
    payload_len += WOL_MAC_LEN;
  }

  // Use broadcast address so the sleeping target receives the packet.
  uint8_t dst_mac[WOL_MAC_LEN];
  memset(dst_mac, WOL_SYNC_BYTE, sizeof(dst_mac));

  uint8_t src_mac[WOL_MAC_LEN];
  err = esp_eth_ioctl(eth, ETH_CMD_G_MAC_ADDR, src_mac);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get source MAC: %s", esp_err_to_name(err));
    return err;
  }

  // Assemble Ethernet frame header and append WoL payload.
  uint8_t frame[(2 * WOL_MAC_LEN) + sizeof(uint16_t) + sizeof(payload)];
  size_t frame_len = 0;

  memcpy(frame + frame_len, dst_mac, WOL_MAC_LEN);
  frame_len += WOL_MAC_LEN;

  memcpy(frame + frame_len, src_mac, WOL_MAC_LEN);
  frame_len += WOL_MAC_LEN;

  frame[frame_len++] = (uint8_t)((WOL_ETHERTYPE >> 8) & 0xFF);
  frame[frame_len++] = (uint8_t)(WOL_ETHERTYPE & 0xFF);

  memcpy(frame + frame_len, payload, payload_len);
  frame_len += payload_len;

  err = esp_eth_transmit(eth, frame, frame_len);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_eth_transmit failed: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGI(TAG, "WoL magic packet sent");

  return ESP_OK;
}
