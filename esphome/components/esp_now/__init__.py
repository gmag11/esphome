# import re

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import automation

# from esphome.automation import Condition
# from esphome.components import logger
from esphome.const import (
    CONF_ADDRESS,
    # CONF_AVAILABILITY,
    # CONF_BIRTH_MESSAGE,
    # CONF_BROKER,
    # CONF_CERTIFICATE_AUTHORITY,
    CONF_CHANNEL,
    # CONF_CLIENT_ID,
    # CONF_COMMAND_TOPIC,
    # CONF_COMMAND_RETAIN,
    # CONF_DISCOVERY,
    # CONF_DISCOVERY_PREFIX,
    # CONF_DISCOVERY_RETAIN,
    # CONF_DISCOVERY_UNIQUE_ID_GENERATOR,
    # CONF_DISCOVERY_OBJECT_ID_GENERATOR,
    # CONF_ID,
    # CONF_FORMAT,
    # CONF_KEEPALIVE,
    # CONF_LEVEL,
    # CONF_LOG_TOPIC,
    CONF_ON_JSON_MESSAGE,
    CONF_ON_MESSAGE,
    # CONF_ON_CONNECT,
    # CONF_ON_DISCONNECT,
    # CONF_PASSWORD,
    CONF_PAYLOAD,
    # CONF_PAYLOAD_AVAILABLE,
    # CONF_PAYLOAD_NOT_AVAILABLE,
    # CONF_PORT,
    # CONF_QOS,
    # CONF_REBOOT_TIMEOUT,
    # CONF_RETAIN,
    # CONF_SHUTDOWN_MESSAGE,
    # CONF_SSL_FINGERPRINTS,
    # CONF_STATE_TOPIC,
    # CONF_TOPIC,
    # CONF_TOPIC_PREFIX,
    CONF_TRIGGER_ID,
    # CONF_USE_ABBREVIATIONS,
    # CONF_USERNAME,
    CONF_WIFI,
    # CONF_WILL_MESSAGE,
)

# from esphome.core import coroutine_with_priority, CORE
# from esphome.components.esp32 import add_idf_sdkconfig_option

# DEPENDENCIES = ["network"]

AUTO_LOAD = ["json"]

# CONF_IDF_SEND_ASYNC = "idf_send_async"
# CONF_SKIP_CERT_CN_CHECK = "skip_cert_cn_check"
CONF_IFACE = "espnow_interface"
CONF_RSSI = "espnow_rssi"
CONF_ESP_NOW = "esp_now"

esp_now_ns = cg.esphome_ns.namespace("CONF_ESP_NOW")
EspNowSendAction = esp_now_ns.class_("EspNowSendAction", automation.Action)
EspNowMessageTrigger = esp_now_ns.class_(
    "EspNowMessageTrigger", automation.Trigger.template(cg.const_char_ptr), cg.Component
)
EspNowComponent = esp_now_ns.class_("EspNowComponent", cg.Component)


def validate_config(value):
    # Populate default fields
    out = value.copy()
    return out


CONF_ESP_IFACE_OPTIONS = ["STA", "AP"]


def validate_esp_iface(value):
    value = cv.string_strict(value)
    if value not in CONF_ESP_IFACE_OPTIONS:
        raise cv.Invalid("Esp-Now interface should be either AP ot STA")
    return value


wifi_channel = cv.int_range(min=0, max=13)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(EspNowComponent),
            cv.Optional(
                CONF_IFACE, default=CONF_ESP_IFACE_OPTIONS[0]
            ): validate_esp_iface,
            cv.Optional(CONF_CHANNEL): wifi_channel,
            cv.Optional(CONF_ON_MESSAGE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(EspNowMessageTrigger),
                    cv.Optional(CONF_RSSI): cv.int_range(-128, 30),
                    cv.Optional(CONF_PAYLOAD): cv.string,
                }
            ),
            cv.Optional(CONF_ON_JSON_MESSAGE): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(EspNowMessageTrigger),
                    cv.Optional(CONF_RSSI): cv.string_strict,
                }
            ),
        }
    ),
    validate_config,
    cv.only_on(["esp32", "esp8266"]),
)


ESP_NOW_SEND_ACTION_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(EspNowComponent),
        cv.Required(CONF_ADDRESS): cv.templatable(cv.mac_address),
        cv.Required(CONF_PAYLOAD): cv.templatable(cv.string),
    }
)


def _final_validate(_):
    try:
        esp_now_channel = fv.full_config.get()[CONF_ESP_NOW][CONF_CHANNEL]
        has_channel = esp_now_channel is not None
    except KeyError:
        has_channel = False

    try:
        wifi_conf = fv.full_config.get()[CONF_WIFI]
        has_wifi = wifi_conf is not None
    except KeyError:
        has_wifi = False

    if has_wifi and has_channel:
        raise cv.Invalid(
            "If wifi is used esp-now will use same channel. So, channel must not be declared into esp-now"
        )


FINAL_VALIDATE_SCHEMA = _final_validate

# @automation.register_action(
#     "esp_now.publish", EspNowSendAction, ESP_NOW_SEND_ACTION_SCHEMA
# )
# async def register_mqtt_component(var, config):
#     await cg.register_component(var, {})

#     if CONF_RETAIN in config:
#         cg.add(var.set_retain(config[CONF_RETAIN]))
#     if not config.get(CONF_DISCOVERY, True):
#         cg.add(var.disable_discovery())
#     if CONF_STATE_TOPIC in config:
#         cg.add(var.set_custom_state_topic(config[CONF_STATE_TOPIC]))
#     if CONF_COMMAND_TOPIC in config:
#         cg.add(var.set_custom_command_topic(config[CONF_COMMAND_TOPIC]))
#     if CONF_COMMAND_RETAIN in config:
#         cg.add(var.set_command_retain(config[CONF_COMMAND_RETAIN]))
#     if CONF_AVAILABILITY in config:
#         availability = config[CONF_AVAILABILITY]
#         if not availability:
#             cg.add(var.disable_availability())
#         else:
#             cg.add(
#                 var.set_availability(
#                     availability[CONF_TOPIC],
#                     availability[CONF_PAYLOAD_AVAILABLE],
#                     availability[CONF_PAYLOAD_NOT_AVAILABLE],
#                 )
#             )


# @automation.register_condition(
#     "mqtt.connected",
#     MQTTConnectedCondition,
#     cv.Schema(
#         {
#             cv.GenerateID(): cv.use_id(MQTTClientComponent),
#         }
#     ),
# )
# async def mqtt_connected_to_code(config, condition_id, template_arg, args):
#     paren = await cg.get_variable(config[CONF_ID])
#     return cg.new_Pvariable(condition_id, template_arg, paren)
