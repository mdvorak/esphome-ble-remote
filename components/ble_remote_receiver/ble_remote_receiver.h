#pragma once

#include "esphome/components/ble_remote_common/ble_remote_common.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/core/automation.h"
#include "esphome/core/component.h"

#include <array>
#include <string>
#include <vector>

namespace esphome::ble_remote {

// Trigger for BLE Remote Receiver on_command event
class BLERemoteReceiverCommandTrigger : public Trigger<uint32_t> {
public:
  explicit BLERemoteReceiverCommandTrigger(class BLERemoteReceiver *parent);

  void set_command(uint16_t command) {
    this->command_ = command;
    this->has_command_ = true;
  }
  uint16_t command() const { return this->command_; }
  bool has_command() const { return this->has_command_; }

protected:
  uint16_t command_{0};
  bool has_command_{false};
};

// BLE Remote Receiver Component
class BLERemoteReceiver : public Component, public esp32_ble_tracker::ESPBTDeviceListener {
public:
  void set_shared_key(const std::string &shared_key) { this->shared_key_.assign(shared_key.begin(), shared_key.end()); }
  void set_mac_address(uint64_t mac_address) { this->mac_address_ = mac_address; }

  void dump_config() override;

  bool parse_device(const esp32_ble_tracker::ESPBTDevice &device) override;

  void add_on_command_trigger(BLERemoteReceiverCommandTrigger *trigger) {
    this->on_command_triggers_.push_back(trigger);
  }

protected:
  static constexpr size_t REPLAY_WINDOW_SIZE = 16;

  bool is_replay_(uint32_t nonce) const;
  void record_nonce_(uint32_t nonce);

  uint64_t mac_address_{0};
  std::vector<uint8_t> shared_key_;
  std::array<uint32_t, REPLAY_WINDOW_SIZE> recent_nonces_{};
  size_t recent_nonces_pos_{0};
  bool initialized_{false};
  std::vector<BLERemoteReceiverCommandTrigger *> on_command_triggers_{};
};

// Implementation of BLERemoteReceiverCommandTrigger constructor
inline BLERemoteReceiverCommandTrigger::BLERemoteReceiverCommandTrigger(BLERemoteReceiver *parent) {
  parent->add_on_command_trigger(this);
}

} // namespace esphome::ble_remote
