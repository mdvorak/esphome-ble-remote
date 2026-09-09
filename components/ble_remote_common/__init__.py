import esphome.config_validation as cv

CODEOWNERS = ["@mdvorak"]
AUTO_LOAD = ["hmac_sha256"]

CONF_SHARED_KEY = "shared_key"
CONF_COMMAND = "command"

SHARED_KEY_SCHEMA = cv.sensitive(cv.All(cv.string, cv.Length(min=8)))
