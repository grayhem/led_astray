#pragma once

#include <FastLED.h>


class Strip {
  public:
    Strip (CRGB* leds, uint16_t len): _leds (leds), _len (len) {};
    ~Strip () = default;

    CRGB* get_leds () {
      return _leds;
    }

    uint16_t get_len () {
      return _len;
    }

    void fill_alternate (CRGB color) {
      for (size_t i = 0; i < _len; i += 2) {
        _leds[i] = color;
      }
    }

  private:
    CRGB* _leds;
    uint16_t _len;
};
