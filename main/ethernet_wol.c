#include "ethernet_wol.h"

#include <string.h>

#include "esp_err.h"
#include "esp_eth_driver.h"
#include "esp_log.h"

#include "common.h"
#include "ethernet_iface.h"

#define WOL_MAC_LEN 6
#define WOL_SYNC_BYTES 6
#define WOL_MAC_REPEAT_COUNT 16
#define WOL_ETHERTYPE 0x0842
#define WOL_SYNC_BYTE 0xFF
#define WOL_PAYLOAD_LEN (WOL_SYNC_BYTES + (WOL_MAC_REPEAT_COUNT * WOL_MAC_LEN))
#define WOL_HEADER_LEN ((2 * WOL_MAC_LEN) + sizeof(uint16_t))

// Ethernet broadcast MAC (FF:FF:FF:FF:FF:FF) for WoL frames.
static const uint8_t kWolBroadcastMac[WOL_MAC_LEN] = {
    WOL_SYNC_BYTE, WOL_SYNC_BYTE, WOL_SYNC_BYTE,
    WOL_SYNC_BYTE, WOL_SYNC_BYTE, WOL_SYNC_BYTE};

esp_err_t ethernet_wol_send(void) {
  esp_eth_handle_t eth = ethernet_iface_get_handle();
  if (eth == NULL) {
    ESP_LOGE(TAG, "Ethernet handle not initialised");
    return ESP_ERR_INVALID_STATE;
  }

  // Build WoL payload: sync bytes followed by repeated broadcast MAC.
  uint8_t payload[WOL_PAYLOAD_LEN];
  memset(payload, WOL_SYNC_BYTE, WOL_SYNC_BYTES);

  uint8_t *payload_cursor = payload + WOL_SYNC_BYTES;
  for (size_t i = 0; i < WOL_MAC_REPEAT_COUNT; ++i) {
    memcpy(payload_cursor, kWolBroadcastMac, WOL_MAC_LEN);
    payload_cursor += WOL_MAC_LEN;
  }

  uint8_t src_mac[WOL_MAC_LEN];
  esp_err_t err = esp_eth_ioctl(eth, ETH_CMD_G_MAC_ADDR, src_mac);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get source MAC: %s", esp_err_to_name(err));
    return err;
  }

  // Assemble Ethernet frame header and append WoL payload.
  uint8_t frame[WOL_HEADER_LEN + WOL_PAYLOAD_LEN];
  size_t frame_len = 0;

  memcpy(frame + frame_len, kWolBroadcastMac, WOL_MAC_LEN);
  frame_len += WOL_MAC_LEN;

  memcpy(frame + frame_len, src_mac, WOL_MAC_LEN);
  frame_len += WOL_MAC_LEN;

  frame[frame_len++] = (uint8_t)((WOL_ETHERTYPE >> 8) & 0xFF);
  frame[frame_len++] = (uint8_t)(WOL_ETHERTYPE & 0xFF);

  memcpy(frame + frame_len, payload, sizeof(payload));
  frame_len += sizeof(payload);

  err = esp_eth_transmit(eth, frame, frame_len);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_eth_transmit failed: %s", esp_err_to_name(err));
    return err;
  }

  ESP_LOGI(TAG, "WoL magic packet sent");

  return ESP_OK;
}
