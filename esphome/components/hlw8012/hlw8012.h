#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/pulse_counter/pulse_counter_sensor.h"

namespace esphome {
  namespace hlw8012 {

    enum HLW8012InitialMode { HLW8012_INITIAL_MODE_CURRENT = 0, HLW8012_INITIAL_MODE_VOLTAGE };

    enum HLW8012SensorModels {
      HLW8012_SENSOR_MODEL_HLW8012 = 0,
      HLW8012_SENSOR_MODEL_CSE7759,
      HLW8012_SENSOR_MODEL_BL0937
    };

#ifdef HAS_PCNT
#define USE_PCNT true
#else
#define USE_PCNT false
#endif

    class HLW8012Component: public PollingComponent {
    public:
      HLW8012Component ()
        : cf_store_ (*pulse_counter::get_storage (USE_PCNT)), cf1_store_ (*pulse_counter::get_storage (USE_PCNT)) {}

      void setup () override;
      void dump_config () override;
      float get_setup_priority () const override;
      void update () override;

      void set_initial_mode (HLW8012InitialMode initial_mode) {
        current_mode_ = initial_mode == HLW8012_INITIAL_MODE_CURRENT;
      }
      void set_sensor_model (HLW8012SensorModels sensor_model) { sensor_model_ = sensor_model; }
      void set_change_mode_every (uint32_t change_mode_every) { change_mode_every_ = change_mode_every; }
      void set_sensor_report_interval (uint32_t report_interval) { report_interval_ = report_interval; }
      // void set_voltage_cycles (uint32_t cycles) { total_voltage_cycles_ = cycles; }
      // void set_current_cycles (uint32_t cycles) { total_current_cycles_ = cycles; }

      void set_current_resistor (float current_resistor) { current_resistor_ = current_resistor; }
      void set_voltage_divider (float voltage_divider) { voltage_divider_ = voltage_divider; }
      void set_voltage_multiplier (float voltage_multiplier) { voltage_multiplier_ = voltage_multiplier; }
      void set_current_multiplier (float current_multiplier) { current_multiplier_ = current_multiplier; }
      void set_power_multiplier (float power_multiplier) { power_multiplier_ = power_multiplier; }
      void set_sel_pin (GPIOPin* sel_pin) { sel_pin_ = sel_pin; }
      void set_cf_pin (InternalGPIOPin* cf_pin) { cf_pin_ = cf_pin; }
      void set_cf1_pin (InternalGPIOPin* cf1_pin) { cf1_pin_ = cf1_pin; }
      void set_voltage_sensor (sensor::Sensor* voltage_sensor) { voltage_sensor_ = voltage_sensor; }
      void set_current_sensor (sensor::Sensor* current_sensor) { current_sensor_ = current_sensor; }
      void set_power_sensor (sensor::Sensor* power_sensor) { power_sensor_ = power_sensor; }
      void set_apparent_power_sensor (sensor::Sensor* apparent_power_sensor) { apparent_power_sensor_ = apparent_power_sensor; }
      void set_reactive_power_sensor (sensor::Sensor* reactive_power_sensor) { reactive_power_sensor_ = reactive_power_sensor; }
      void set_power_factor_sensor (sensor::Sensor* power_factor_sensor) { power_factor_sensor_ = power_factor_sensor; }
      void set_energy_sensor (sensor::Sensor* energy_sensor) { energy_sensor_ = energy_sensor; }
      void set_calibration (bool enabled) { calibration_enabled_ = enabled; }
      void set_calibration_voltage (float voltage) { calibration_voltage_ = voltage; }
      void set_calibration_current (float current) { calibration_current_ = current; }
      void set_calibration_power (float power) { calibration_power_ = power; }

    protected:
      uint32_t nth_value_{ 0 };
      bool current_mode_{ false };
      uint32_t change_mode_at_{ 0 };
      uint32_t change_mode_every_{ 3 };
      uint32_t report_interval_{ 60 * 1000 };
      // uint32_t total_voltage_cycles_{ 2 };
      // uint32_t total_current_cycles_{ 4 };
      float current_resistor_{ 0.001 };
      float voltage_divider_{ 2351 };
      HLW8012SensorModels sensor_model_{ HLW8012_SENSOR_MODEL_HLW8012 };
      uint64_t cf_total_pulses_{ 0 };
      GPIOPin* sel_pin_;
      InternalGPIOPin* cf_pin_;
      pulse_counter::PulseCounterStorageBase& cf_store_;
      InternalGPIOPin* cf1_pin_;
      pulse_counter::PulseCounterStorageBase& cf1_store_;
      sensor::Sensor* voltage_sensor_{ nullptr };
      sensor::Sensor* current_sensor_{ nullptr };
      sensor::Sensor* power_sensor_{ nullptr };
      sensor::Sensor* apparent_power_sensor_{ nullptr };
      sensor::Sensor* reactive_power_sensor_{ nullptr };
      sensor::Sensor* power_factor_sensor_{ nullptr };
      sensor::Sensor* energy_sensor_{ nullptr };

      float voltage_multiplier_{ 0.0f };
      float current_multiplier_{ 0.0f };
      float power_multiplier_{ 0.0f };

      bool calibration_enabled_{ false };
      float calibration_voltage_{ 230.0 };
      float calibration_current_{ 0.26 };
      float calibration_power_{ 60.0 };

    };

  }  // namespace hlw8012
}  // namespace esphome
