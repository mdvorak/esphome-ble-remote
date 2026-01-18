#define XXH_INLINE_ALL
#define XXH_NO_STDLIB
#define XXH_NO_XXH3
#define XXH_NO_LONG_LONG

#include "ble_remote_common.h"
#include "xxhash.h"

using namespace esphome::ble_remote;

uint32_t esphome::ble_remote::ble_remote_calculate_hash(const BLERemoteCommandData &data, const std::vector<uint8_t> &shared_key) {
  const uint8_t* data_bytes = reinterpret_cast<const uint8_t*>(&data);

  XXH32_state_s state;
  XXH32_reset(&state, 42);
  XXH32_update(&state, data_bytes, sizeof(BLERemoteCommandData) - sizeof(uint32_t));
  XXH32_update(&state, shared_key.data(), shared_key.size());
  return XXH32_digest(&state);;
}

bool esphome::ble_remote::ble_remote_validate_data(const BLERemoteCommandData &data, const std::vector<uint8_t> &shared_key) {
  return data.nonce != 0 && data.hash == ble_remote_calculate_hash(data, shared_key);
}
