#include "ble_remote_common.h"

#include <cstddef>
#include <cstring>

#if defined(USE_ESP32)
#include <psa/crypto.h>
#else
#include "mbedtls/md.h"
#endif

namespace esphome::ble_remote {

uint64_t ble_remote_calculate_hash(const BLERemoteCommandData &data, const std::vector<uint8_t> &shared_key) {
  const uint8_t *data_bytes = reinterpret_cast<const uint8_t *>(&data);
  const size_t data_len = offsetof(BLERemoteCommandData, hash);

  // HMAC-SHA256 over (command || nonce), keyed with shared_key. Truncated to 8 bytes for wire format.
  uint8_t digest[32]{};

#if defined(USE_ESP32)
  psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
  psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
  psa_set_key_algorithm(&attributes, PSA_ALG_HMAC(PSA_ALG_SHA_256));
  psa_set_key_type(&attributes, PSA_KEY_TYPE_HMAC);

  mbedtls_svc_key_id_t key_id = MBEDTLS_SVC_KEY_ID_INIT;
  if (psa_import_key(&attributes, shared_key.data(), shared_key.size(), &key_id) == PSA_SUCCESS) {
    size_t mac_len = 0;
    psa_mac_compute(key_id, PSA_ALG_HMAC(PSA_ALG_SHA_256), data_bytes, data_len, digest, sizeof(digest), &mac_len);
    psa_destroy_key(key_id);
  }
#else
  const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  mbedtls_md_hmac(md_info, shared_key.data(), shared_key.size(), data_bytes, data_len, digest);
#endif

  uint64_t result;
  std::memcpy(&result, digest, sizeof(result));
  return result;
}

} // namespace esphome::ble_remote
