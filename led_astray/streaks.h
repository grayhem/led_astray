#pragma once

// start a trail at a random location in a random direction, with a random duration.

// max number of random streaks active at once
// with 30 streaks at max length and not overlapping with each other, that's 339 LEDs at full brightness.
// bump that up to 500 to account for trails.
// at 0.02A, that's 10A.
// figure that we're actually going to see 7/8 of that for now because one pin won't be hooked up to anything.
// anecdotally i think we drew less than that with original 1100 LEDs. got more than 10 hours i think. maybe 16.
// 5/17/23 did a test and battery A ran for 12 hours. so a little more than 8A if the battery is all the way happy.
// that's with the mixed program. two modes.
#define NUM_STREAKS 3

// we'll only run the streaks half the time
#define STREAK_INTERVAL_S 666

struct StreakParameters {
  int min_length = 5;    // support size
  int max_length = 13;
  int min_travel = 20;   // distance the streak can travel, in pixels
  int max_travel = 100;
  uint16_t streak_chance = 5;  // likelihood, per frame, of starting one streak. 1000 = always, 0 = never
  uint8_t max_speed = 60;      // higher = faster. movement and palette shift.
  uint8_t min_speed = 5;
};


struct Streak {
  int head;       // current head
  int tail;       // current tail
  int start;      // where the head began
  int stop;       // when tail passes this, deactivate
  bool reverse;   // moving from high to low pixel index
  bool active;    // currently drawing
  int pin;                    // it lives on this pin
  uint8_t speed;              // every frame, subtract this from the movement counter
  uint8_t movement_counter;   // when this crosses 0, advance one pixel. and advance palette index.
  int palette_index;
  int palette_increment;      // roll palette forwards or backwards

  void update () {
    if (speed >= movement_counter) {
      palette_index++;
      if (reverse) {
        head = max (stop, head-1);
        tail = max (stop, tail-1);
      } else {
        head = min (stop, head+1);
        tail = min (stop, tail+1);
      }
      active = (tail != stop);
    }
    movement_counter -= speed;
  }
};

template<int num_pins, int num_streaks>
class StreakFarm {
public:
  StreakFarm (Strip* strip, CRGBPalette16* palette, StreakParameters* parameters) {
    _strip = strip;
    _leds = _strip->get_leds ();
    _palette = palette;
    _parameters = parameters;
    _num_leds = _strip->get_len ();
    for (size_t i = 0; i < num_streaks; ++i) {
      _streaks[i] = new Streak ();
    }
 }

  ~StreakFarm () = default;


  // when this function is called, start a timer. after the timer runs out,
  // stop spawning new streaks.
  void start_counter (int runtime_s) {
    _run_until_ms = millis () + runtime_s * 1000;
  }



  void advance_palette (int s = 1) {
    _palette_offset += s;
  }

  // go through all the streaks. if active, update. if inactive, try to activate.
  void update_streaks () {
    for (int i=0; i < num_streaks; ++i) {
      Streak* streak = _streaks[i];
      if (streak->active) {
        draw (streak);
        streak->update ();
      } else if ((random16 (1000) < _parameters->streak_chance) & (millis () < _run_until_ms)) {
          initialize (streak);
      }
    }
  }

private:
  Strip* _strip;
  CRGB*          _leds;
  CRGBPalette16* _palette;

  Streak*   _streaks[num_streaks];

  StreakParameters* _parameters;

  int _palette_offset = 0;
  int _support;
  int _num_leds;

  int _run_until_ms = 0;
  int _counter = 0;

  // just roll palette backwards if the streak is moving backwards
  void draw (Streak* streak) {
    int palette_index = streak->palette_index + _palette_offset;
    if (streak->reverse) {
      for (int i = streak->head; i < streak->tail; ++i) {
        int absolute = streak->pin * _num_leds + i;
        palette_index += streak->palette_increment;
        _leds[absolute] = ColorFromPalette(
          *_palette,
          palette_index % 240,
          255);
      }
    } else {
      for (int i = streak->tail; i < streak->head; ++i) {
        int absolute = streak->pin * _num_leds + i;
        palette_index += streak->palette_increment;
        _leds[absolute] = ColorFromPalette(
          *_palette,
          palette_index % 240,
          255);
      }
    }
    
  }

  void initialize (Streak* streak) {
    streak->pin = random (num_pins);
    streak->reverse = random8 () % 2;
    streak->speed = random8 (_parameters->min_speed, _parameters->max_speed);
    streak->palette_index = random8 ();
    int length = random16 (_parameters->min_length, _parameters->max_length);
    int travel = random16 (_parameters->min_travel, _parameters->max_travel);
    if (streak->reverse) {
      streak->head = random16 (travel, _num_leds - length);
      streak->tail = streak->head + length;
      streak->stop = streak->head - travel;
      streak->palette_increment = -1;
    } else {
      streak->head = random16 (length, _num_leds - travel);
      streak->tail = streak->head - length;
      streak->stop = streak->head + travel;
      streak->palette_increment = 1;
    }
    streak->start = streak->head;
    streak->active = true;
    streak->movement_counter = 255;
  }
};




