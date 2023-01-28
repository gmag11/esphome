#include "hlw8012.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace hlw8012 {

    static const char* const TAG = "hlw8012";

    // valid for HLW8012 and CSE7759
    static const uint32_t HLW8012_CLOCK_FREQUENCY = 3579000;

    void HLW8012Component::setup () {
      float reference_voltage = 0;
      ESP_LOGCONFIG (TAG, "Setting up HLW8012...");
      this->sel_pin_->setup ();
      this->sel_pin_->digital_write (this->current_mode_);
      this->cf_store_.pulse_counter_setup (this->cf_pin_);
      this->cf1_store_.pulse_counter_setup (this->cf1_pin_);

      // Initialize multipliers
      if (this->sensor_model_ == HLW8012_SENSOR_MODEL_BL0937) {
        reference_voltage = 1.218f;
        if (this->power_multiplier_ <= 0) {
          this->power_multiplier_ =
            reference_voltage * reference_voltage * this->voltage_divider_ / this->current_resistor_ / 1721506.0f;
        }
        if (this->current_multiplier_ <= 0) {
          this->current_multiplier_ = reference_voltage / this->current_resistor_ / 94638.0f;
        }
        if (voltage_multiplier_ <= 0) {
          this->voltage_multiplier_ = reference_voltage * this->voltage_divider_ / 15397.0f;
        }
      } else {
        // HLW8012 and CSE7759 have same reference specs
        reference_voltage = 2.43f;
        if (this->power_multiplier_ <= 0) {
          this->power_multiplier_ = reference_voltage * reference_voltage * this->voltage_divider_ / this->current_resistor_ *
            64.0f / 24.0f / HLW8012_CLOCK_FREQUENCY; // 10,343611735 // 10343611
        }
        if (this->current_multiplier_ <= 0) {
          this->current_multiplier_ = reference_voltage / this->current_resistor_ * 512.0f / 24.0f / HLW8012_CLOCK_FREQUENCY; // 0,408636513 // 408636
        }
        if (voltage_multiplier_ <= 0) {

          this->voltage_multiplier_ = reference_voltage * this->voltage_divider_ * 256.0f / HLW8012_CLOCK_FREQUENCY; // 0,014484493 // 14484
        }
      }
    }

    void HLW8012Component::dump_config () {
      ESP_LOGCONFIG (TAG, "HLW8012:");
      LOG_PIN ("  SEL Pin: ", this->sel_pin_);
      LOG_PIN ("  CF Pin: ", this->cf_pin_);
      LOG_PIN ("  CF1 Pin: ", this->cf1_pin_);
      ESP_LOGCONFIG (TAG, "  Change measurement mode every %u", this->change_mode_every_);
      // ESP_LOGCONFIG (TAG, "  Voltage cycles %u", this->total_voltage_cycles_);
      // ESP_LOGCONFIG (TAG, "  Current cycles %u", this->total_current_cycles_);
      ESP_LOGCONFIG (TAG, "  Current resistor: %.1f mΩ", this->current_resistor_ * 1000.0f);
      ESP_LOGCONFIG (TAG, "  Voltage Divider: %.1f", this->voltage_divider_);
      ESP_LOGCONFIG (TAG, "  Voltage Multiplier: %.6f", this->voltage_multiplier_);
      ESP_LOGCONFIG (TAG, "  Current Multiplier: %.6f", this->current_multiplier_);
      ESP_LOGCONFIG (TAG, "  Power Multiplier: %.6f", this->power_multiplier_);
      ESP_LOGCONFIG (TAG, "  CALIBRATION ENABLED: %s", this->calibration_enabled_ ? "YES" : "NO");
      ESP_LOGCONFIG (TAG, "  Calibration voltage: %.0f", this->calibration_voltage_);
      ESP_LOGCONFIG (TAG, "  Calibration current: %.0f", this->calibration_current_);
      ESP_LOGCONFIG (TAG, "  Calibration power: %.0f", this->calibration_power_);
      LOG_UPDATE_INTERVAL (this);
      LOG_SENSOR ("  ", "Voltage", this->voltage_sensor_);
      LOG_SENSOR ("  ", "Current", this->current_sensor_);
      LOG_SENSOR ("  ", "Power", this->power_sensor_);
      LOG_SENSOR ("  ", "Energy", this->energy_sensor_);
    }

    float HLW8012Component::get_setup_priority () const {
      return setup_priority::DATA;
    }

    void HLW8012Component::update () {

      static float current = 0;
      static float voltage = 0;
      static float power = 0;
      static float apparent_power = 0;
      static float reactive_power = 0;
      static float power_factor = 0;

      static uint32_t last_report = 0;

      // HLW8012 has 50% duty cycle
      pulse_counter::pulse_counter_t raw_cf = this->cf_store_.read_raw_value ();
      pulse_counter::pulse_counter_t raw_cf1 = this->cf1_store_.read_raw_value ();
      float cf_hz = raw_cf / (this->get_update_interval () / 1000.0f);
      if (raw_cf <= 1) {
        // don't count single pulse as power
        cf_hz = 0.0f;
      }
      ESP_LOGD (TAG, "raw_cf: %d, cf_hz: %f", raw_cf, cf_hz);
      float cf1_hz = raw_cf1 / (this->get_update_interval () / 1000.0f);
      if (raw_cf1 <= 1) {
        // don't count single pulse as anything
        cf1_hz = 0.0f;
      }
      ESP_LOGD (TAG, "raw_cf1: %d, cf1_hz: %f", raw_cf1, cf1_hz);

      if (this->nth_value_++ < 2) {
        return;
      }

      power = cf_hz * this->power_multiplier_;

      if (calibration_enabled_ && power > 0) {
        power_multiplier_ *= ((double)calibration_power_ / power);
        ESP_LOGI (TAG, "[CALIBRATION] Calculated power multiplier %.6f", power_multiplier_);
      }

      ESP_LOGV (TAG, "Cycle %u. Mode: %s", change_mode_at_, current_mode_ ? "CURRENT" : "VOLTAGE");

      // Only read cf1 after one cycle. Apparently it's quite unstable after being changed.
      if (this->change_mode_at_ != 0) {
        if (this->current_mode_) {
          current = cf1_hz * this->current_multiplier_;
          if (calibration_enabled_ && current > 0) {
            current_multiplier_ *= ((double)calibration_current_ / current);
            ESP_LOGI (TAG, "[CALIBRATION] Calculated current multiplier %.6f", current_multiplier_);
          }
          ESP_LOGD (TAG, "Got power=%.1fW, current=%.1fA", power, current);
          // if (this->current_sensor_ != nullptr) {
          //   this->current_sensor_->publish_state (current);
          // }
        } else {
          voltage = cf1_hz * this->voltage_multiplier_;
          if (calibration_enabled_ && voltage > 0) {
            voltage_multiplier_ *= ((double)calibration_voltage_ / voltage);
            ESP_LOGI (TAG, "[CALIBRATION] Calculated voltage multiplier %.6f", voltage_multiplier_);
          }
          ESP_LOGD (TAG, "Got power=%.1fW, voltage=%.1fV", power, voltage);
          // if (this->voltage_sensor_ != nullptr) {
          //   this->voltage_sensor_->publish_state (voltage);
          // }
        }
      }

      apparent_power = voltage * current;
      reactive_power = sqrt (apparent_power * apparent_power - power * power);
      if (apparent_power > 0) {
        power_factor = (double)power / apparent_power;
      } else {
        power_factor = 0;
      }

      if (millis () - last_report >= this->report_interval_) {

        ESP_LOGD (TAG, "Sending sensors");
        last_report = millis ();

        if (this->current_sensor_ != nullptr) {
          this->current_sensor_->publish_state (current);
        }

        if (this->voltage_sensor_ != nullptr) {
          this->voltage_sensor_->publish_state (voltage);
        }

        if (this->power_sensor_ != nullptr) {
          this->power_sensor_->publish_state (power);
        }

        if (this->apparent_power_sensor_ != nullptr) {
          this->apparent_power_sensor_->publish_state (apparent_power);
        }

        if (this->power_factor_sensor_ != nullptr) {
          this->power_factor_sensor_->publish_state (power_factor * 100);
        }

        if (this->energy_sensor_ != nullptr) {
          cf_total_pulses_ += raw_cf;
          float energy = cf_total_pulses_ * this->power_multiplier_ / 3600;
          this->energy_sensor_->publish_state (energy);
        }
      }

      // TODO: Add reactive power

      // if (this->current_mode_) {
      if (this->change_mode_at_++ == this->change_mode_every_) {
        this->current_mode_ = !this->current_mode_;
        ESP_LOGV (TAG, "Changing mode to %s mode", this->current_mode_ ? "CURRENT" : "VOLTAGE");
        this->change_mode_at_ = 0;
        this->sel_pin_->digital_write (this->current_mode_);
      }
      // } else {
      //   ESP_LOGV (TAG, "Voltage Mode");
      //   if (this->change_mode_at_++ == this->total_voltage_cycles_) {
      //     this->current_mode_ = !this->current_mode_;
      //     ESP_LOGV (TAG, "Changing mode to %s mode", this->current_mode_ ? "CURRENT" : "VOLTAGE");
      //     this->change_mode_at_ = 0;
      //     this->sel_pin_->digital_write (this->current_mode_);
      //   }
      // }
    }  // HLW8012Component::update
  }  // namespace hlw8012
}  // namespace esphome
