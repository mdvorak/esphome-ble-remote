import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import esp32_ble_server
from esphome.const import CONF_ID
from esphome.components.ble_remote_common import CONF_SHARED_KEY, CONF_COMMAND, SHARED_KEY_SCHEMA

CODEOWNERS = ["@mdvorak"]
DEPENDENCIES = ["esp32_ble_server"]
AUTO_LOAD = ["ble_remote_common"]

ble_remote_ns = cg.esphome_ns.namespace("ble_remote")
BLERemote = ble_remote_ns.class_("BLERemote", cg.Component)
BLERemoteWriteAction = ble_remote_ns.class_("BLERemoteWriteAction", automation.Action)

CONF_BLE_SERVER_ID = "ble_server_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BLERemote),
        cv.GenerateID(CONF_BLE_SERVER_ID): cv.use_id(esp32_ble_server.BLEServer),
        cv.Required(CONF_SHARED_KEY): SHARED_KEY_SCHEMA,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    ble_server = await cg.get_variable(config[CONF_BLE_SERVER_ID])
    cg.add(var.set_ble_server(ble_server))
    cg.add(var.set_shared_key(config[CONF_SHARED_KEY]))


@automation.register_action(
    "ble_remote.write",
    BLERemoteWriteAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(BLERemote),
            cv.Required(CONF_COMMAND): cv.hex_uint16_t,
        }
    ),
    synchronous=True,
)
async def ble_remote_write_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cg.add(var.set_command(config[CONF_COMMAND]))
    return var
