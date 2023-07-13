#pragma once


/*
a firefly gets a random color from the current palette when it spawns.
a firefly turns off briefly when it moves. while it is lit, it stays put.
fireflies do not have collision and they are not aware of their neighbors.
normally, only a fraction of the total number of fireflies will get to update. 
the firefarm update method can be called with a boost factor to increase the 
number of fireflies that can be spawned. a factor of 0 (default) sets the number
to the soft cap in parameters, and a factor of 255 (max) sets it to NUM_FIREFLIES.
*/

// this is a hard limit on the number of fireflies that can ever be active simultaneously.
// there will usually be fewer than this active.
#define NUM_FIREFLIES 250

struct Firefly {
  CRGB color;               // it is (currently) this color
  bool live = false;        // is it alive
  bool lit;                 // is it shining
  int relight_ms;           // when will it shine again
  int pin;                  // it lives on this pin
  int position;             // where it lives on the pin
  uint32_t expiration_ms;   // when it will die ;_;
  uint8_t activity;         // how likely it is to move on a given frame
  int range;                // how far it could potentially move in one step
  int slowness;             // how long it goes dark during movement, in milliseconds per index moved
};

struct FireFarmParameters {
  int min_duration_ms = 5000;
  int max_duration_ms = 15000;
  uint8_t min_activity = 1;
  uint8_t max_activity = 10;
  int min_range = 1;
  int max_range = 10;
  int min_slowness = 100;
  int max_slowness = 2000;
  uint16_t spawn_chance = 3;    // out of 1000, how likely a given dead firefly is to spawn on a given frame.
  size_t soft_cap = 100;        // when not in boost mode, max out at this many fireflies
  uint8_t palette_fuzz = 35;    // when picking a color for a new firefly, this controls its randomness
};

template<int num_pins, int num_fireflies>
class FireFarm {
public:
  FireFarm (Strip* strip, CRGBPalette16* palette, FireFarmParameters* parameters) {
    _strip = strip;
    _leds = _strip->get_leds ();
    _palette = palette;
    _num_leds = _strip->get_len ();
    _parameters = parameters;
    for (size_t i = 0; i < num_fireflies; ++i) {
      _fireflies[i] = new Firefly ();
    }
  }

  ~FireFarm () = default;

  void advance_palette (int s = 1) {
    _palette_offset += s;
  }

  // boost_factor increases the number of fireflies that can be alive at once.
  // bonus_spawn increases the likelihood for any dead firefly to start going.
  void update (uint8_t boost_factor = 0, uint16_t bonus_spawn = 0) {
    //Serial.print ("live count: ");
    //Serial.println (_live_count);
    size_t spawn_cap = map (boost_factor, 0, 255, _parameters->soft_cap, num_fireflies);
    for (size_t i = 0; i < num_fireflies; ++i) {
      Firefly* firefly = _fireflies[i];
      if (firefly->live) {
        draw (firefly);
        update_firefly (firefly);
      } else if (_live_count < spawn_cap and (random16 (1000) < (_parameters->spawn_chance + bonus_spawn))) {
          initialize (firefly);
      }
    }
  }


private:

  void update_firefly (Firefly* firefly) {
    firefly->lit = millis () > firefly->relight_ms;
    if (millis () > firefly->expiration_ms) {
      firefly->live = false;
      --_live_count;
    } else if (firefly->lit and random8 () < firefly->activity) {
      int displacement = random (firefly->range * 2) - firefly->range;
      int new_position = firefly->position + displacement;
      new_position = min (new_position, _num_leds - 1);
      new_position = max (new_position, 0);
      // correct the displacement to its actual value
      displacement = abs (new_position - firefly->position);
      firefly->lit = false;
      firefly->position = new_position;
      firefly->relight_ms = millis () + firefly->slowness * displacement;
    } 
  }

  void initialize (Firefly* firefly) {
    ++_live_count;
    firefly->pin = random8 (num_pins);
    firefly->position = random16 (_num_leds);
    uint8_t palette_index = (_palette_offset + random8 (_parameters->palette_fuzz)) % 255;
    firefly->color = ColorFromPalette (*_palette, palette_index);
    firefly->live = true;
    firefly->lit = true;
    firefly->expiration_ms = millis () + random16 (_parameters->min_duration_ms, _parameters->max_duration_ms);
    firefly->activity = random8 (_parameters->min_activity, _parameters->max_activity);
    firefly->range = random8 (_parameters->min_range, _parameters->max_range);
    firefly->slowness = random (_parameters->min_slowness, _parameters->max_slowness);
  }

  void draw (Firefly* firefly) {
    if (firefly->lit) {
      int absolute = firefly->pin * _num_leds + firefly->position;
      _leds[absolute] = firefly->color;
    } 
  }



  Strip* _strip;
  CRGB*          _leds;
  CRGBPalette16* _palette;

  Firefly* _fireflies[num_fireflies];

  FireFarmParameters* _parameters;

  int _palette_offset = 0;
  int _num_leds;
  size_t _live_count = 0;
};











