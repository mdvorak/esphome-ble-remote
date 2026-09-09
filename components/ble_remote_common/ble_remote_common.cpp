#include "ble_remote_common.h"
#include "esphome/components/hmac_sha256/hmac_sha256.h"

#include <cstddef>
#include <cstring>

namespace esphome::ble_remote {

uint64_t BLERemoteHMACKey::calculate_hash(const BLERemoteCommandData &data) const {
  const uint8_t *data_bytes = reinterpret_cast<const uint8_t *>(&data);
  const size_t data_len = offsetof(BLERemoteCommandData, hash);

  hmac_sha256::HmacSHA256 hmac;
  hmac.init(this->key_.data(), this->key_.size());
  hmac.add(data_bytes, data_len);
  hmac.calculate();

  uint8_t digest[32];
  hmac.get_bytes(digest);

  uint64_t truncated = 0;
  std::memcpy(&truncated, digest, sizeof(truncated));
  return truncated;
}

} // namespace esphome::ble_remote
