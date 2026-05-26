# Changelog

## [1.0.1](https://github.com/mdvorak/esphome-ble-remote/compare/v1.0.0...v1.0.1) (2026-05-26)


### Bug Fixes

* use PSA crypto exclusively, drop mbedTLS fallback, call psa_crypto_init ([0074c0c](https://github.com/mdvorak/esphome-ble-remote/commit/0074c0c493a3e79645c01f0c8bac6a2aab25af59))
* wrap PSA HMAC key in BLERemoteHMACKey class ([5eda6b1](https://github.com/mdvorak/esphome-ble-remote/commit/5eda6b1125fd1d36f3b5438ee65a3479fdc71932))

## 1.0.0 (2026-05-26)


### ⚠ BREAKING CHANGES

* new updated version

### Features

* initial version ([134fd8c](https://github.com/mdvorak/esphome-ble-remote/commit/134fd8c1a82e9ff54bc4f30db0e00c77f7b7fee4))
* new updated version ([3c9ded8](https://github.com/mdvorak/esphome-ble-remote/commit/3c9ded8b23e1e435e5d75cc72feb73b1b87f7779))
* refactor CI matrix, simplify PSA crypto, bump min version to 2026.5.0 ([d406004](https://github.com/mdvorak/esphome-ble-remote/commit/d40600459f8269c0db30833734bdae81fda66be7))
