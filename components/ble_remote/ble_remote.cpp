#include "ble_remote.h"
#include "esp_random.h"
#include "esphome/core/log.h"

namespace esphome::ble_remote {

static constexpr char TAG[] = "ble_remote";

void BLERemote::dump_config() {
  ESP_LOGCONFIG(TAG, "BLE Remote:");
  ESP_LOGCONFIG(TAG, "  Shared Key: %u bytes", shared_key_.size());
}

void BLERemote::write(uint16_t command) {
  // Avoid zero nonce, which is considered invalid (unlikely but possible)
  uint32_t nonce;
  do {
    nonce = esp_random();
  } while (nonce == 0);

  std::vector<uint8_t> bytes(sizeof(BLERemoteCommandData));

  BLERemoteCommandData* data = reinterpret_cast<BLERemoteCommandData*>(bytes.data());
  data->command = command;
  data->nonce = nonce;
  data->hash = ble_remote_calculate_hash(*data, this->shared_key_);

  ESP_LOGD(TAG, "Writing BLE remote command 0x%04X nonce %u hash: %u", command, nonce, data->hash);
  this->ble_server_->set_manufacturer_data(bytes);
}

}  // namespace esphome::ble_remote
