#include "espnow_basic.h"

namespace esphome {
  namespace esp_now {

    ESPNOWComponent* esp_now_component = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    ESPNOWComponent::ESPNOWComponent () {
      esp_now_component = this;
    }

    void ESPNOWComponent::set_channel (int espnow_channel) {
      if (espnow_channel >= MIN_WIFI_CHANNEL && espnow_channel <= MAX_WIFI_CHANNEL){
        channel = espnow_channel;
      }
    }

    int ESPNOWComponent::get_channel () {
      return channel;
    }

    void ESPNOWComponent::dataReceived (uint8_t* address, uint8_t* data, uint8_t len, signed int rssi, bool broadcast) {
      Serial.print ("Received: ");
      Serial.printf ("%.*s\n", len, data);
      Serial.printf ("RSSI: %d dBm\n", rssi);
      Serial.printf ("From: " MACSTR "\n", MAC2STR (address));
      Serial.printf ("%s\n", broadcast ? "Broadcast" : "Unicast");
    }

    void ESPNOWComponent::setup () {
      //quickEspNow.onDataRcvd
#if defined ESP32 && defined ESP32_COMPAT_MODE
      quickEspNow.setWiFiBandwidth (WIFI_IF_STA, WIFI_BW_HT20); // Only needed for ESP32 in case you need coexistence with ESP8266 in the same network
#endif //ESP32 && ESP32_COMPAT_MODE
      quickEspNow.begin (channel);
    }

  }
}
