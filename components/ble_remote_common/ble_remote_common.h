#pragma once

#include <stdint.h>
#include <vector>

namespace esphome::ble_remote {

// 0xFFFF is reserved by the Bluetooth SIG for testing/development use, so it's
// the safe choice to fill the BLE manufacturer-data Company Identifier field
// without colliding with a real registered vendor.
static constexpr uint16_t BLE_REMOTE_COMPANY_ID = 0xFFFF;

struct BLERemoteCommandData {
  uint16_t command{0};
  uint32_t nonce{0};  // Random value for deduplication
  uint64_t hash{0};
} __attribute__((packed));

uint64_t ble_remote_calculate_hash(const BLERemoteCommandData &data, const std::vector<uint8_t> &shared_key);

}  // namespace esphome::ble_remote
