#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/ble_remote_common/ble_remote_common.h"

#include <string>
#include <vector>

namespace esphome::ble_remote {

// Trigger for BLE Remote Receiver on_toggle event
class BLERemoteReceiverToggleTrigger : public Trigger<uint32_t> {
 public:
  explicit BLERemoteReceiverToggleTrigger(class BLERemoteReceiver *parent);

  void set_command(uint16_t command) { this->command_ = command; this->has_command_ = true; }
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

  void add_on_toggle_trigger(BLERemoteReceiverToggleTrigger *trigger) {
    this->on_toggle_triggers_.push_back(trigger);
  }

 protected:
  uint64_t mac_address_{0};
  std::vector<uint8_t> shared_key_;
  uint32_t last_nonce_{0};
  uint32_t last_received_nonce_{0};
  bool initialized_{false};
  std::vector<BLERemoteReceiverToggleTrigger *> on_toggle_triggers_{};
};

// Implementation of BLERemoteReceiverToggleTrigger constructor
inline BLERemoteReceiverToggleTrigger::BLERemoteReceiverToggleTrigger(BLERemoteReceiver *parent) {
  parent->add_on_toggle_trigger(this);
}

}  // namespace esphome::ble_remote
