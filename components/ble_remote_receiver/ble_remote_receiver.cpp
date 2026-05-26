#include "ble_remote_receiver.h"
#include "esphome/core/log.h"

#include <cinttypes>
#include <cstring>
#include <optional>

namespace esphome::ble_remote {

static constexpr char TAG[] = "ble_remote";

void BLERemoteReceiver::dump_config() {
  ESP_LOGCONFIG(TAG, "BLE Remote Receiver:");
  ESP_LOGCONFIG(TAG, "  Remote MAC Address: %012llX", this->mac_address_);
  ESP_LOGCONFIG(TAG, "  Shared Key: %u bytes", this->hmac_key_.key_size());
}

static std::optional<BLERemoteCommandData> parse_ble_remote_command_data(const esp32_ble_tracker::ServiceData &data) {
  const auto data_uuid = data.uuid.get_uuid();
  if (data_uuid.len != 2 || data_uuid.uuid.uuid16 != BLE_REMOTE_COMPANY_ID ||
      data.data.size() != sizeof(BLERemoteCommandData)) {
    return std::nullopt;
  }

  BLERemoteCommandData result{};
  std::memcpy(&result, data.data.data(), sizeof(BLERemoteCommandData));
  return result;
}

bool BLERemoteReceiver::is_replay_(uint32_t nonce) const {
  for (uint32_t recent : this->recent_nonces_) {
    if (recent == nonce) {
      return true;
    }
  }
  return false;
}

void BLERemoteReceiver::record_nonce_(uint32_t nonce) {
  this->recent_nonces_[this->recent_nonces_pos_] = nonce;
  this->recent_nonces_pos_ = (this->recent_nonces_pos_ + 1) % REPLAY_WINDOW_SIZE;
}

bool BLERemoteReceiver::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  // Check if MAC address matches
  if (device.address_uint64() != this->mac_address_) {
    return false;
  }

  for (const auto &data : device.get_manufacturer_datas()) {
    auto parsed = parse_ble_remote_command_data(data);
    if (!parsed) {
      ESP_LOGD(TAG, "Unable to parse BLE remote command data: uuid len %u data size %u", data.uuid.get_uuid().len,
               data.data.size());
      continue;
    }
    const auto &command_data = *parsed;

    // Authenticate before touching any receiver state.
    const auto expected_hash = this->hmac_key_.calculate_hash(command_data);
    if (command_data.hash != expected_hash) {
      ESP_LOGD(TAG, "Invalid BLE remote command data: command 0x%04X nonce %" PRIu32 " hash %016llX",
               command_data.command, command_data.nonce, command_data.hash);
      continue;
    }

    // Boot sentinel: transmitter broadcasts nonce=0 from setup() until the first real press.
    // Use it to mark ourselves initialized (so the next real packet triggers) and never fire
    // a command from the sentinel itself.
    if (command_data.nonce == 0) {
      if (!this->initialized_) {
        this->initialized_ = true;
        ESP_LOGD(TAG, "Received boot sentinel; ready");
      }
      return true;
    }

    // Replay protection: skip if we have seen this nonce recently. This also covers the common
    // case of identical advertisement-train repeats from the same transmission.
    if (this->is_replay_(command_data.nonce)) {
      continue;
    }
    this->record_nonce_(command_data.nonce);

    // Fallback init-skip: if we boot while the transmitter is still broadcasting a real
    // command (non-zero nonce), absorb it once instead of replaying that pre-reboot press.
    if (!this->initialized_) {
      this->initialized_ = true;
      ESP_LOGD(TAG, "Initialized with nonce %" PRIu32, command_data.nonce);
      return true;
    }

    ESP_LOGD(TAG, "Command 0x%04X with nonce %" PRIu32, command_data.command, command_data.nonce);
    for (auto *trigger : this->on_command_triggers_) {
      if (!trigger->has_command() || trigger->command() == command_data.command) {
        trigger->trigger(command_data.command);
      }
    }
    return true;
  }

  // MAC matched even if no valid command in this advertisement; claim the device.
  return true;
}

} // namespace esphome::ble_remote
