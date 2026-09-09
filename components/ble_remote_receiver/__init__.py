import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import ble_device_base
from esphome.const import CONF_ID, CONF_MAC_ADDRESS, CONF_TRIGGER_ID
from esphome.components.ble_remote_common import CONF_SHARED_KEY, CONF_COMMAND, SHARED_KEY_SCHEMA

CODEOWNERS = ["@mdvorak"]
AUTO_LOAD = ["ble_device_base", "ble_remote_common"]

ble_remote_ns = cg.esphome_ns.namespace("ble_remote")
BLERemoteReceiver = ble_remote_ns.class_(
    "BLERemoteReceiver", cg.Component, ble_device_base.ESPBTDeviceListener
)
BLERemoteReceiverCommandTrigger = ble_remote_ns.class_(
    "BLERemoteReceiverCommandTrigger", automation.Trigger.template(cg.uint32)
)

CONF_ON_COMMAND = "on_command"

CONFIG_SCHEMA = cv.All(
    ble_device_base.rename_legacy_hub_id("ble_remote_receiver"),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BLERemoteReceiver),
            cv.Required(CONF_MAC_ADDRESS): cv.mac_address,
            cv.Required(CONF_SHARED_KEY): SHARED_KEY_SCHEMA,
            cv.Optional(CONF_ON_COMMAND): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        BLERemoteReceiverCommandTrigger
                    ),
                    cv.Optional(CONF_COMMAND): cv.hex_uint16_t,
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ble_device_base.BLE_DEVICE_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_device_base.register_ble_device(var, config)

    cg.add(var.set_mac_address(config[CONF_MAC_ADDRESS].as_hex))
    cg.add(var.set_shared_key(config[CONF_SHARED_KEY]))

    for conf in config.get(CONF_ON_COMMAND, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        if CONF_COMMAND in conf:
            cg.add(trigger.set_command(conf[CONF_COMMAND]))
        await automation.build_automation(trigger, [(cg.uint32, "x")], conf)
