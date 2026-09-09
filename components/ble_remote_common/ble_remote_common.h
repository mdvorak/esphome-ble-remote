#pragma once

#include <cstddef>
#include <stdint.h>
#include <utility>
#include <vector>

namespace esphome::ble_remote {

// 0xFFFF is reserved by the Bluetooth SIG for testing/development use, so it's
// the safe choice to fill the BLE manufacturer-data Company Identifier field
// without colliding with a real registered vendor.
static constexpr uint16_t BLE_REMOTE_COMPANY_ID = 0xFFFF;

struct BLERemoteCommandData {
  uint16_t command{0};
  uint32_t nonce{0}; // Random value for deduplication
  uint64_t hash{0};
} __attribute__((packed));

class BLERemoteHMACKey {
public:
  void setup(std::vector<uint8_t> key) { this->key_ = std::move(key); }
  // HMAC-SHA256 over the struct up to (excluding) the hash field, truncated to
  // its leftmost 8 bytes. The truncation matches the on-air format and must not
  // change; see the wire format table in the README.
  uint64_t calculate_hash(const BLERemoteCommandData &data) const;
  size_t key_size() const { return this->key_.size(); }

private:
  std::vector<uint8_t> key_;
};

} // namespace esphome::ble_remote
