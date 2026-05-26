#include "ble_remote.h"
#include "esp_random.h"
#include "esphome/core/log.h"
#include <inttypes.h>

#include <cstring>

namespace esphome::ble_remote {

static constexpr char TAG[] = "ble_remote";

void BLERemote::setup() { this->send_boot_sentinel_(); }

void BLERemote::dump_config() {
  ESP_LOGCONFIG(TAG, "BLE Remote:");
  ESP_LOGCONFIG(TAG, "  BLE Server: %s", this->ble_server_ != nullptr ? "attached" : "missing");
  ESP_LOGCONFIG(TAG, "  Shared Key: %u bytes", this->hmac_key_.key_size());
}

void BLERemote::send_boot_sentinel_() {
  if (this->ble_server_ == nullptr) {
    return;
  }

  BLERemoteCommandData data{};
  data.command = 0;
  data.nonce = 0;
  data.hash = this->hmac_key_.calculate_hash(data);

  std::vector<uint8_t> bytes(sizeof(uint16_t) + sizeof(BLERemoteCommandData));
  bytes[0] = static_cast<uint8_t>(BLE_REMOTE_COMPANY_ID & 0xFF);
  bytes[1] = static_cast<uint8_t>((BLE_REMOTE_COMPANY_ID >> 8) & 0xFF);
  std::memcpy(bytes.data() + sizeof(uint16_t), &data, sizeof(BLERemoteCommandData));

  ESP_LOGD(TAG, "Broadcasting boot sentinel (nonce 0)");
  this->ble_server_->set_manufacturer_data(bytes);
}

void BLERemote::write(uint16_t command) {
  if (this->ble_server_ == nullptr) {
    ESP_LOGW(TAG, "BLE server not configured; dropping command 0x%04X", command);
    return;
  }

  // Generate a fresh nonce. Avoid 0 (treated as sentinel elsewhere) and avoid colliding with the
  // immediately-previous nonce (would be silently dedup'd by the receiver's replay window).
  uint32_t nonce;
  do {
    nonce = esp_random();
  } while (nonce == 0 || nonce == this->last_nonce_);
  this->last_nonce_ = nonce;

  BLERemoteCommandData data{};
  data.command = command;
  data.nonce = nonce;
  data.hash = this->hmac_key_.calculate_hash(data);

  // Wire format: [Company ID 0xFFFF (2 B)] [BLERemoteCommandData (14 B)].
  std::vector<uint8_t> bytes(sizeof(uint16_t) + sizeof(BLERemoteCommandData));
  bytes[0] = static_cast<uint8_t>(BLE_REMOTE_COMPANY_ID & 0xFF);
  bytes[1] = static_cast<uint8_t>((BLE_REMOTE_COMPANY_ID >> 8) & 0xFF);
  std::memcpy(bytes.data() + sizeof(uint16_t), &data, sizeof(BLERemoteCommandData));

  ESP_LOGD(TAG, "Writing BLE remote command 0x%04X nonce %" PRIu32 " hash: %016llX", command, nonce, data.hash);
  this->ble_server_->set_manufacturer_data(bytes);
}

} // namespace esphome::ble_remote
