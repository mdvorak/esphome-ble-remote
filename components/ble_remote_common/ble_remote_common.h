#pragma once

#include <stdint.h>
#include <vector>

namespace esphome::ble_remote {

struct BLERemoteCommandData {
  uint16_t command{0};
  uint32_t nonce{0};  // Random value for deduplication
  uint32_t hash{0};
} __attribute__((packed));

uint32_t ble_remote_calculate_hash(const BLERemoteCommandData &data, const std::vector<uint8_t> &shared_key);

bool ble_remote_validate_data(const BLERemoteCommandData &data, const std::vector<uint8_t> &shared_key);

}  // namespace esphome::ble_remote
