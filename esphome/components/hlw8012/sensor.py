import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import pins
from esphome.components import sensor
from esphome.const import (
    CONF_ACTIVE,
    CONF_CHANGE_MODE_EVERY,
    CONF_INITIAL_MODE,
    CONF_CALIBRATION,
    CONF_CURRENT,
    CONF_CURRENT_RESISTOR,
    CONF_ID,
    CONF_POWER,
    CONF_ENERGY,
    CONF_SEL_PIN,
    CONF_MODEL,
    CONF_VOLTAGE,
    CONF_VOLTAGE_DIVIDER,
    CONF_VOLTAGE_MULTIPLIER,
    CONF_CURRENT_MULTIPLIER,
    CONF_APPARENT_POWER,
    CONF_REACTIVE_POWER,
    CONF_POWER_MULTIPLIER,
    CONF_POWER_FACTOR,
    CONF_REPORT_INTERVAL,
    CONF_SENSOR,
    CONF_PLATFORM,
    CONF_UPDATE_INTERVAL,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_APPARENT_POWER,
    DEVICE_CLASS_POWER_FACTOR,
    DEVICE_CLASS_REACTIVE_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_WATT,
    UNIT_WATT_HOURS,
    UNIT_PERCENT,
)

AUTO_LOAD = ["pulse_counter"]

hlw8012_ns = cg.esphome_ns.namespace("hlw8012")
HLW8012Component = hlw8012_ns.class_("HLW8012Component", cg.PollingComponent)
HLW8012InitialMode = hlw8012_ns.enum("HLW8012InitialMode")
HLW8012SensorModels = hlw8012_ns.enum("HLW8012SensorModels")

INITIAL_MODES = {
    CONF_CURRENT: HLW8012InitialMode.HLW8012_INITIAL_MODE_CURRENT,
    CONF_VOLTAGE: HLW8012InitialMode.HLW8012_INITIAL_MODE_VOLTAGE,
}

MODELS = {
    "HLW8012": HLW8012SensorModels.HLW8012_SENSOR_MODEL_HLW8012,
    "CSE7759": HLW8012SensorModels.HLW8012_SENSOR_MODEL_CSE7759,
    "BL0937": HLW8012SensorModels.HLW8012_SENSOR_MODEL_BL0937,
}

CALIBRATION_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ACTIVE): cv.boolean,
        cv.Optional(CONF_POWER, default=60): cv.positive_float,
        cv.Optional(CONF_VOLTAGE, default=230): cv.positive_float,
        cv.Optional(CONF_CURRENT, default=0.26): cv.positive_float,
    }
)

CONF_CF1_PIN = "cf1_pin"
CONF_CF_PIN = "cf_pin"


def validate_report_interval(value):
    value = cv.positive_time_period_milliseconds(value)
    if value < cv.time_period("0s"):
        raise cv.Invalid(
            "HLW8012 report interval must be greater than or equal to 0 seconds if set."
        )
    return value


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HLW8012Component),
        cv.Required(CONF_SEL_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_CF_PIN): cv.All(pins.internal_gpio_input_pullup_pin_schema),
        cv.Required(CONF_CF1_PIN): cv.All(pins.internal_gpio_input_pullup_pin_schema),
        cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_CURRENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_AMPERE,
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_CURRENT,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_APPARENT_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_APPARENT_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_REACTIVE_POWER): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_REACTIVE_POWER,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_POWER_FACTOR): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=2,
            device_class=DEVICE_CLASS_POWER_FACTOR,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_ENERGY): sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT_HOURS,
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_ENERGY,
            state_class=STATE_CLASS_TOTAL_INCREASING,
        ),
        cv.Optional(CONF_CURRENT_RESISTOR, default=0.001): cv.resistance,
        cv.Optional(CONF_VOLTAGE_MULTIPLIER, default=0): cv.positive_float,
        cv.Optional(CONF_CURRENT_MULTIPLIER, default=0): cv.positive_float,
        cv.Optional(CONF_POWER_MULTIPLIER, default=0): cv.positive_float,
        cv.Optional(CONF_VOLTAGE_DIVIDER, default=2351): cv.positive_float,
        cv.Optional(CONF_MODEL, default="HLW8012"): cv.enum(MODELS, upper=True),
        cv.Optional(CONF_CHANGE_MODE_EVERY, default=3): cv.All(
            cv.uint32_t, cv.Range(min=1)
        ),
        # TODO: Check that report interval is higher than update interval
        cv.Optional(CONF_REPORT_INTERVAL, default="60s"): validate_report_interval,
        cv.Optional(CONF_INITIAL_MODE, default=CONF_VOLTAGE): cv.one_of(
            *INITIAL_MODES, lower=True
        ),
        cv.Optional(CONF_CALIBRATION): CALIBRATION_SCHEMA,
    }
).extend(cv.polling_component_schema("5s"))


def _final_validate(_):
    try:
        full_config_sensor = fv.full_config.get()[CONF_SENSOR]
        for sensor_entry in full_config_sensor:
            if sensor_entry[CONF_PLATFORM] == "hlw8012":
                if (
                    sensor_entry[CONF_REPORT_INTERVAL]
                    < sensor_entry[CONF_UPDATE_INTERVAL]
                ):
                    # report interval must be greater or equal than update interval
                    sensor_entry[CONF_REPORT_INTERVAL] = sensor_entry[
                        CONF_UPDATE_INTERVAL
                    ]
                    # raise cv.Invalid(
                    #     "HLW8012 Report interval must be greater or equal than update interval"
                    # )
    except KeyError:
        print("Key error")


FINAL_VALIDATE_SCHEMA = _final_validate


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sel = await cg.gpio_pin_expression(config[CONF_SEL_PIN])
    cg.add(var.set_sel_pin(sel))
    cf = await cg.gpio_pin_expression(config[CONF_CF_PIN])
    cg.add(var.set_cf_pin(cf))
    cf1 = await cg.gpio_pin_expression(config[CONF_CF1_PIN])
    cg.add(var.set_cf1_pin(cf1))

    if CONF_VOLTAGE in config:
        sens = await sensor.new_sensor(config[CONF_VOLTAGE])
        cg.add(var.set_voltage_sensor(sens))
    if CONF_CURRENT in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT])
        cg.add(var.set_current_sensor(sens))
    if CONF_POWER in config:
        sens = await sensor.new_sensor(config[CONF_POWER])
        cg.add(var.set_power_sensor(sens))
    if CONF_APPARENT_POWER in config:
        sens = await sensor.new_sensor(config[CONF_APPARENT_POWER])
        cg.add(var.set_apparent_power_sensor(sens))
    if CONF_REACTIVE_POWER in config:
        sens = await sensor.new_sensor(config[CONF_REACTIVE_POWER])
        cg.add(var.set_reactive_power_sensor(sens))
    if CONF_POWER_FACTOR in config:
        sens = await sensor.new_sensor(config[CONF_POWER_FACTOR])
        cg.add(var.set_power_factor_sensor(sens))
    if CONF_ENERGY in config:
        sens = await sensor.new_sensor(config[CONF_ENERGY])
        cg.add(var.set_energy_sensor(sens))
    cg.add(var.set_current_resistor(config[CONF_CURRENT_RESISTOR]))
    cg.add(var.set_voltage_divider(config[CONF_VOLTAGE_DIVIDER]))
    cg.add(var.set_voltage_multiplier(config[CONF_VOLTAGE_MULTIPLIER]))
    cg.add(var.set_current_multiplier(config[CONF_CURRENT_MULTIPLIER]))
    cg.add(var.set_power_multiplier(config[CONF_POWER_MULTIPLIER]))
    cg.add(var.set_change_mode_every(config[CONF_CHANGE_MODE_EVERY]))
    cg.add(var.set_sensor_report_interval(config[CONF_REPORT_INTERVAL]))
    # cg.add(var.set_voltage_cycles(config[CONF_VOLTAGE_CYCLES]))
    # cg.add(var.set_current_cycles(config[CONF_CURRENT_CYCLES]))
    cg.add(var.set_initial_mode(INITIAL_MODES[config[CONF_INITIAL_MODE]]))
    cg.add(var.set_sensor_model(config[CONF_MODEL]))

    if CONF_CALIBRATION in config:
        cg.add(var.set_calibration(config[CONF_CALIBRATION][CONF_ACTIVE]))
        cg.add(var.set_calibration_voltage(config[CONF_CALIBRATION][CONF_VOLTAGE]))
        cg.add(var.set_calibration_current(config[CONF_CALIBRATION][CONF_CURRENT]))
        cg.add(var.set_calibration_power(config[CONF_CALIBRATION][CONF_POWER]))
