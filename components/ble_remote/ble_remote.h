#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/esp32_ble_server/ble_server.h"
#include "esphome/components/ble_remote_common/ble_remote_common.h"

#include <string>
#include <vector>

namespace esphome::ble_remote {

// BLE Remote Transmitter Component
class BLERemote : public Component {
 public:
  void set_ble_server(esp32_ble_server::BLEServer *server) { this->ble_server_ = server; }
  void set_shared_key(const std::string &shared_key) { this->shared_key_.assign(shared_key.begin(), shared_key.end()); }

  void dump_config() override;

  // Write a command using BLE remote
  void write(uint16_t command);

 protected:
  esp32_ble_server::BLEServer *ble_server_{nullptr};
  std::vector<uint8_t> shared_key_;
};

// Action to write to BLE Remote
template<typename... Ts> class BLERemoteWriteAction : public Action<Ts...>, public Parented<BLERemote> {
 public:
  void set_command(uint16_t command) { this->command_ = command; }

  void play(const Ts &... /*x*/) override { this->parent_->write(this->command_); }

 protected:
  uint16_t command_{0};
};

}  // namespace esphome::ble_remote
