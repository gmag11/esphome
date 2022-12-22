#include "espnow_basic.h"

namespace esphome {
  namespace esp_now {

    ESPNOWComponent* esp_now_component = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    ESPNOWComponent::ESPNOWComponent () {
      esp_now_component = this;
    }

  }
}
