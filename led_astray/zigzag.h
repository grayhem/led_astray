#pragma once

/*
another farm pattern/ pattern farm.
this pattern is a pixel that moves back and forth rapidly over an expanding 
or contracting range.
expanding zigzags go until one end leaves the strip, and contracting zigzags
go until the range collapses.
or until it runs out of time.

this is buggy and seems to light the entire strip
*/

#define NUM_ZIGZAGS 3
#define ZIGZAG_SPAWN_LIKELIHOOD 33   // out of 100
#define ZIGZAG_SPAWN_INTERVAL_S 7    // try this often to spawn a zigzag

void spawn_zigzag () {

}

struct Zigzag {
  bool live = false;
  uint32_t expiration_ms = 0;       // when it will die ;_;
  CRGB color;                       // it is (currently) this color
  uint8_t pin = 0;                  // which pin
  uint16_t position = 0;            // where it is now
  uint8_t move_counter = 0;         // how long until it moves
  uint8_t slowness = 0;             // how many frames it stays on a given pixel
  uint16_t lo = 0;                  // lower limit pixel coordinate
  uint16_t hi = 0;
  uint8_t range_step = 0;           // how much the limit moves each time a limit is hit
  bool expanding = true;            // it can also contract
  bool climbing = true;             // up or down the strip
};

struct ZigzagParameters {
  uint8_t min_slowness = 5;
  uint8_t max_slowness = 15;
  uint8_t min_range_step = 1;
  uint8_t max_range_step = 7;
  uint16_t buffer = 30;             // keep starting range away from edges of strip
  uint16_t min_duration_ms = 5000;
  uint16_t max_duration_ms = 15000;
};

template<int num_pins, int num_zigzags>
class ZigzagFarm {
public:
  ZigzagFarm (Strip* strip, CRGBPalette16* palette, ZigzagParameters* parameters) {
    _strip = strip;
    _leds = _strip->get_leds ();
    _palette = palette;
    _num_leds = _strip->get_len ();
    _parameters = parameters;
    for (size_t i = 0; i < num_zigzags; ++i) {
      _zigzags[i] = new Zigzag ();
    }
  }
  ~ZigzagFarm () = default;

  void advance_palette (int s = 1) {
    _palette_offset += s;
  }

  void spawn () {
    Zigzag* candidate;
    for (size_t i = 0; i < num_zigzags; ++i) {
      candidate = _zigzags[i];
      if (not candidate->live) {
        candidate->live = true;
        candidate->pin = random8 (num_pins);
        uint8_t palette_index = _palette_offset + random8 ();
        candidate->color = ColorFromPalette (*_palette, palette_index);
        candidate->slowness = random8 (_parameters->min_slowness, _parameters->max_slowness);
        candidate->range_step = random8 (_parameters->min_range_step, _parameters->max_range_step);
        candidate->expanding = (random8 (1) == 1);
        candidate->climbing = (random8 (1) == 1);
        candidate->lo = random16 (_parameters->buffer, _num_leds - 2 * _parameters->buffer);
        candidate->hi = random16 (candidate->lo + _parameters->buffer, _num_leds - _parameters->buffer);
        candidate->position = random16 (candidate->lo, candidate->hi);
        candidate->move_counter = candidate->slowness;
        candidate->expiration_ms = millis () + random16 (_parameters->min_duration_ms, _parameters->max_duration_ms);
        Serial.print ("position ");
        Serial.println (candidate->position);
        Serial.print ("expanding ");
        Serial.println (candidate->expanding);
        break;
      }
    }
  }

  void draw () {
    for (size_t i = 0; i < num_zigzags; ++i) {
      Zigzag* client = _zigzags[i];
      // kill it if it ran out of time
      client->live = millis () > client->expiration_ms;
      if (client->live) {
        Serial.print ("zigzag position: ");
        Serial.println (client->position);
        // moving before drawing is slightly uglier control flow.
        // because the zigzag might be killed while moving.
        int absolute = client->pin * _num_leds + client->position;
        _leds[absolute] = client->color;
        _move (client);
      }
    }
  }

private:

  void _move (Zigzag* client) {
    if (client->move_counter) {
      --client->move_counter;
    } else {
      if (client->climbing) {
        ++client->position;
      } else {
        --client->position;
      }
      _modify_range (client);
    }
  }

  void _modify_range (Zigzag* client) {
    uint16_t position = client->position;
    // kill it if it hit the end of the strip
    if (position == 0 or position == _num_leds) {
      client->live = false;
    }
    // contract/ expand the range
    if (position == client->lo) {
      client->climbing = not client->climbing;
      if (client->expanding) {
        client->lo -= client->range_step;
      } else {
        client->lo += client->range_step;
        if (client->lo >= client->hi) {
          // kill it if the range has collapsed
          client->live = false;
        } else {
          // adjust the position or else the zigzag will escape bounds.
          // we include the +1 here so it doesn't trigger the range modify
          // logic on the next move.
          client->position = client->lo + 1;
        }
      }
    } else if (position == client->hi) {
      client->climbing = not client->climbing;
      if (client->expanding) {
        client->hi += client->range_step;
      } else {
        client->hi -= client->range_step;
        if (client->hi <= client->lo) {
          client->live = false;
        } else {
          client->position = client->hi - 1;
        }
      }
    }

/*
      Serial.print ("lo ");
      Serial.println (client->lo);
      Serial.print ("hi ");
      Serial.println (client->hi);
      */
  }

  Strip* _strip;
  CRGB*          _leds;
  CRGBPalette16* _palette;

  Zigzag* _zigzags[num_zigzags];

  ZigzagParameters* _parameters;

  int _palette_offset = 0;
  int _num_leds;
  size_t _live_count = 0;

};




