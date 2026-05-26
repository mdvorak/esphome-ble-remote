#include "ble_remote_common.h"
#include "esphome/core/log.h"

#include <cstddef>

namespace esphome::ble_remote {

static constexpr char TAG[] = "ble_remote";
static constexpr psa_algorithm_t HMAC_ALG = PSA_ALG_TRUNCATED_MAC(PSA_ALG_HMAC(PSA_ALG_SHA_256), sizeof(uint64_t));

void BLERemoteHMACKey::setup(const std::vector<uint8_t> &key) {
  this->key_size_ = key.size();

  psa_crypto_init();

  psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
  psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
  psa_set_key_algorithm(&attributes, HMAC_ALG);
  psa_set_key_type(&attributes, PSA_KEY_TYPE_HMAC);

  psa_destroy_key(this->key_id_);
  this->key_id_ = MBEDTLS_SVC_KEY_ID_INIT;

  psa_status_t status = psa_import_key(&attributes, key.data(), key.size(), &this->key_id_);
  if (status != PSA_SUCCESS) {
    ESP_LOGE(TAG, "psa_import_key failed: %d", (int)status);
  }
}

uint64_t BLERemoteHMACKey::calculate_hash(const BLERemoteCommandData &data) const {
  const uint8_t *data_bytes = reinterpret_cast<const uint8_t *>(&data);
  const size_t data_len = offsetof(BLERemoteCommandData, hash);

  uint64_t digest = 0;
  size_t mac_len = 0;
  psa_status_t status = psa_mac_compute(this->key_id_, HMAC_ALG, data_bytes, data_len,
                                        reinterpret_cast<uint8_t *>(&digest), sizeof(digest), &mac_len);
  if (status != PSA_SUCCESS) {
    ESP_LOGW(TAG, "psa_mac_compute failed: %d", (int)status);
  }

  return digest;
}

} // namespace esphome::ble_remote
