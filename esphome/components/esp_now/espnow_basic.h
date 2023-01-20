#pragma once

#include "Arduino.h"
#include "esphome/core/defines.h"

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/json/json_util.h"
#include <cstddef>
#include <vector>

#ifdef USE_ESP32_FRAMEWORK_ARDUINO
#include <esp_wifi.h>
#include <WiFiType.h>
#include <WiFi.h>
#endif

#ifdef USE_ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WiFiType.h>

#if defined(USE_ESP8266) && USE_ARDUINO_VERSION_CODE < VERSION_CODE(2, 4, 0)
extern "C" {
#include <user_interface.h>
};
#endif
#endif

#include <QuickDebug.h>
#include <QuickEspNow.h>

namespace esphome {
  namespace esp_now {

    // using comms_hal_rcvd_data;
    // using comms_hal_sent_data;

    class ESPNOWComponent: public Component {
    public:
      ESPNOWComponent ();
      void setup () override;
      void dump_config () override {}
      void loop () override {}
      void on_shutdown () override {}
      float get_setup_priority () const override { return 40; }
      bool can_proceed () override { return true; }
      int32_t send (const uint8_t* dstAddress, const uint8_t* payload, size_t payload_len) { return 0; }
      void set_channel (int espnow_channel);
      int get_channel ();

      using on_message_callback_t = void (uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast);

      void add_on_message_callback (std::function<on_message_callback_t>&& callback);

      // void set_on_message (std::function<on_message_callback_t>&& callback) {
      //   auto async_callback = [callback] (uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast) {
      //     callback (address, data, len, rssi, broadcast);
      //   };
      //   quickEspNow.onDataRcvd (std::move (async_callback));
      // }
      // void on_message_cb (uint8_t* address, std::vector<uint8_t>& data, int rssi, bool broadcast);

    protected:
      int channel = CURRENT_WIFI_CHANNEL;
      std::vector<uint8_t> payload_buffer_;
      void dataReceived (uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast);
      CallbackManager<on_message_callback_t> message_callback_{};
    };

    extern ESPNOWComponent* esp_now_component;

    class ESPNOWMessageTrigger: public Trigger<std::string>, public Component {
    public:
      explicit ESPNOWMessageTrigger () {}
      void set_payload (const std::string& payload) {}
      void setup () override {}
      void dump_config () override {}
      float get_setup_priority () const override { return 40; }
      void set_min_rssi (int rssi) {
        if (rssi >= -128 || rssi <= 30) {
          min_rssi = rssi;
        }
      }

    protected:
      optional<uint8_t> payload_[250];
      size_t payload_len = 0;
      uint8_t srcAddress[6];
      int min_rssi = -128;
    };

    template<typename... Ts> class ESPNOWSendAction: public Action<Ts...> {
    public:
      ESPNOWSendAction (ESPNOWComponent* parent): parent_ (parent) {}
      TEMPLATABLE_VALUE (uint8_t[250], payload_)
        TEMPLATABLE_VALUE (size_t, payload_len_)
        TEMPLATABLE_VALUE (uint8_t[6], dstAddress_)

        void play (Ts... x) override {
        this->parent_->send (this->dstAddress_.value (x...), this->payload_.value (x...), this->payload_len_.value (x...));
      }

      void set_payload (const std::string& payload) {}
      void set_address (const uint8_t addr[6]) {}

    protected:
      ESPNOWComponent* parent_;
      uint8_t dstAddress[6];
      uint8_t payload_[250];
      size_t payload_len = 0;

    };
  }
}
