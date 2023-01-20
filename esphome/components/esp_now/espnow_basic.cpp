#include "espnow_basic.h"
#include <functional>

namespace esphome {
  namespace esp_now {

    static const char* const TAG = "esp-now";

    ESPNOWComponent* esp_now_component = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    ESPNOWComponent::ESPNOWComponent () {
      esp_now_component = this;
    }

    void ESPNOWComponent::set_channel (int espnow_channel) {
      if (espnow_channel >= MIN_WIFI_CHANNEL && espnow_channel <= MAX_WIFI_CHANNEL) {
        channel = espnow_channel;
      }
    }

    int ESPNOWComponent::get_channel () {
      return channel;
    }

    void ESPNOWComponent::dataReceived (uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast) {
      ESP_LOGI (TAG, "Received: %.*s\n", len, data);
      ESP_LOGI (TAG, "RSSI: %d dBm\n", rssi);
      ESP_LOGI (TAG, "From: " MACSTR "\n", MAC2STR (address));
      ESP_LOGI (TAG, "%s\n", broadcast ? "Broadcast" : "Unicast");

      this->message_callback_.call (address, data, len, rssi, broadcast);
    }

    using namespace std::placeholders;

    void ESPNOWComponent::setup () {
      quickEspNow.onDataRcvd (std::bind (&ESPNOWComponent::dataReceived, this, _1, _2, _3, _4, _5));
#if defined ESP32 && defined ESP32_COMPAT_MODE
      quickEspNow.setWiFiBandwidth (WIFI_IF_STA, WIFI_BW_HT20); // Only needed for ESP32 in case you need coexistence with ESP8266 in the same network
#endif //ESP32 && ESP32_COMPAT_MODE
      quickEspNow.begin (channel);
    }

    void ESPNOWComponent::add_on_message_callback (std::function<on_message_callback_t>&& callback) {
      this->message_callback_.add (std::move (callback));
    }

  }
}
