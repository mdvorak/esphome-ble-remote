#include "ble_remote_receiver.h"
#include "esphome/core/log.h"

namespace esphome::ble_remote {

static constexpr char TAG[] = "ble_remote";

static BLERemoteCommandData parse_ble_remote_command_data(const esp32_ble_tracker::ServiceData& data) {
  BLERemoteCommandData result;

  const auto data_uuid = data.uuid.get_uuid();
  if (data_uuid.len != 2 || data.data.size() != sizeof(BLERemoteCommandData) - sizeof(uint16_t)) {
    return result;
  }

  result.command = data_uuid.uuid.uuid16;
  result.nonce = *reinterpret_cast<const uint32_t*>(&data.data[0]);
  result.hash = *reinterpret_cast<const uint32_t*>(&data.data[sizeof(uint32_t)]);
  return result;
}

void BLERemoteReceiver::dump_config() {
  ESP_LOGCONFIG(TAG, "BLE Remote Receiver:");
  ESP_LOGCONFIG(TAG, "  Remote MAC Address: %012llX", this->mac_address_);
  ESP_LOGCONFIG(TAG, "  Shared Key: %u bytes", shared_key_.size());
}

bool BLERemoteReceiver::parse_device(const esp32_ble_tracker::ESPBTDevice &device) {
  // Check if MAC address matches
  if (device.address_uint64() != this->mac_address_) {
    return false;
  }

  // Look for manufacturer data
  for (auto data : device.get_manufacturer_datas()) {
    const auto command_data = parse_ble_remote_command_data(data);
    const auto last_received_nonce = this->last_received_nonce_;
    this->last_received_nonce_ = command_data.nonce;

    if (!ble_remote_validate_data(command_data, this->shared_key_)) {
      // Provide intelligent feedback
      if (command_data.nonce != last_received_nonce) {
        if (command_data.nonce != 0) {
          ESP_LOGD(TAG, "Invalid BLE remote command data: command 0x%04X nonce %u hash %u", command_data.command, command_data.nonce, command_data.hash);
        } else {
          ESP_LOGD(TAG, "Unable to parse BLE remote command data: uuid len %u data size %u", data.uuid.get_uuid().len, data.data.size());
        }
      }
      continue;
    }

    if (!this->initialized_) {
      this->last_nonce_ = command_data.nonce;
      this->initialized_ = true;
      ESP_LOGD(TAG, "Initialized with nonce %u", command_data.nonce);
    } else if (command_data.nonce != this->last_nonce_) {
      this->last_nonce_ = command_data.nonce;
      ESP_LOGD(TAG, "Command 0x%04X with nonce %u", command_data.command, command_data.nonce);

      // Trigger on_toggle automations for matching command
      for (auto *trigger : this->on_toggle_triggers_) {
        if (!trigger->has_command() || trigger->command() == command_data.command) {
          trigger->trigger(command_data.command);
        }
      }
      return true;
    }
  }

  return false;
}

}  // namespace esphome::ble_remote
