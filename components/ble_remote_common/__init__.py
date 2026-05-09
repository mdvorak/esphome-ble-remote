import esphome.config_validation as cv

CODEOWNERS = ["@mdvorak"]

CONF_SHARED_KEY = "shared_key"
CONF_COMMAND = "command"

SHARED_KEY_SCHEMA = cv.All(cv.string, cv.Length(min=8))
