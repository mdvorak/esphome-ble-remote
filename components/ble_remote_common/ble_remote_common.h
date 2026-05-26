#pragma once

#include <psa/crypto.h>
#include <stdint.h>
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
  BLERemoteHMACKey() = default;
  ~BLERemoteHMACKey() { psa_destroy_key(this->key_id_); }

  BLERemoteHMACKey(const BLERemoteHMACKey &) = delete;
  BLERemoteHMACKey &operator=(const BLERemoteHMACKey &) = delete;

  void setup(const std::vector<uint8_t> &key);
  uint64_t calculate_hash(const BLERemoteCommandData &data) const;
  size_t key_size() const { return key_size_; }

private:
  mbedtls_svc_key_id_t key_id_{MBEDTLS_SVC_KEY_ID_INIT};
  size_t key_size_{0};
};

} // namespace esphome::ble_remote
