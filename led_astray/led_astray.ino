// run multiple patterns (fireflies, streaks, zigzag) that vary in activity over time.
// zigzag wasn't tested fully before firefly and had to be turned off at the event.

#include <OctoWS2811.h>
#include <FastLED.h>
#include <Arduino.h>

#include "ugh.h"


#include "led_parameters.h"
#include "controller.h"
#include "palettes.h"
#include "strip.h"
#include "streaks.h"
#include "timer.h"
#include "fire.h"
#include "zigzag.h"


TeensyController<RGB, WS2811_800kHz>* controller;

// pins for the leds
byte pins[NUM_PINS] = {2, 14, 7, 8, 6, 20, 21, 5};

// These buffers need to be large enough for all the pixels.
// The total number of pixels is "NUM_LEDS * NUM_PINS".
// Each pixel needs 3 bytes, so multiply by 3.  An "int" is
// 4 bytes, so divide by 4.  The array is created using "int"
// so the compiler will align it to 32 bit memory.
DMAMEM int display_memory[ALL_LEDS * 3 / 4];
int        drawing_memory[ALL_LEDS * 3 / 4];
OctoWS2811 octo (
  NUM_LEDS, 
  display_memory, 
  drawing_memory, 
  WS2811_RGB | WS2811_800kHz, 
  NUM_PINS, 
  pins);


CRGB rgbarray[ALL_LEDS];
Strip* strip;



uint8_t noise[ALL_LEDS];
uint8_t bright_noise[ALL_LEDS];
static uint16_t noise_seed;

// controls blend from one palette to next
PaletteBlender* blender_a;
int blend_rate = 24;
CRGBPalette16* palette_a;
PaletteBlender* blender_b;
CRGBPalette16* palette_b;
PaletteBlender* blender_c;
CRGBPalette16* palette_c;


// index into current palette. contributes to visual smoothness in some patterns
int palette_start_index = 0;


//MultiSlider<NUM_PINS, NUM_TRAILS>* slider;


FireFarm<NUM_PINS, NUM_FIREFLIES>* firefarm;
FireFarmParameters* firefarm_parameters;

ZigzagFarm<NUM_PINS, NUM_ZIGZAGS>* zigzagfarm;
ZigzagParameters* zigzag_parameters;

StreakFarm<NUM_PINS, NUM_STREAKS>* streaks;
StreakParameters* streak_parameters;


Timer* timer;

uint8_t blur_amount = 0;


void setup()
{
  Serial.begin(38400);
  pinMode (LED_BUILTIN, OUTPUT);
  digitalWrite (LED_BUILTIN, HIGH);      
  octo.begin ();
  controller = new TeensyController<RGB, WS2811_800kHz> (&octo);



  FastLED.setBrightness (BRIGHTNESS);

  FastLED.addLeds (controller, rgbarray, ALL_LEDS).setCorrection (Typical8mmPixel);
  fill_solid (rgbarray, ALL_LEDS, CRGB (0, 0, 0));         
  FastLED.show ();      

  // note that we're telling the strip object it has 300 LEDs instead of the total 2400!!!
  strip = new Strip (rgbarray, NUM_LEDS);
  noise_seed = random16 ();

  timer = new Timer (3);

    

  int color_threshold = 192;
  int sparkle_threshold = 220;
  int scale = 10;
  int x_step = 1;

  blender_a = new PaletteBlender (blend_rate);
  palette_a = blender_a->get_palette ();
  blender_b = new PaletteBlender (blend_rate);
  palette_b = blender_b->get_palette ();
  blender_c = new PaletteBlender (blend_rate);
  palette_c = blender_c->get_palette ();

  zigzag_parameters = new ZigzagParameters ();
  zigzagfarm = new ZigzagFarm<NUM_PINS, NUM_ZIGZAGS> (strip, palette_b, zigzag_parameters);

  firefarm_parameters = new FireFarmParameters ();
  firefarm = new FireFarm<NUM_PINS, NUM_FIREFLIES> (strip, palette_a, firefarm_parameters);
  
  streak_parameters = new StreakParameters ();
  streaks = new StreakFarm<NUM_PINS, NUM_STREAKS> (strip, palette_c, streak_parameters);
}

void run_firefarm () {
  blender_a->blend_palette ();
  EVERY_N_SECONDS (PALETTE_DURATION_S) {
    blender_a->next ();
  }
  EVERY_N_MILLIS (50) {
    firefarm->advance_palette (1);
  }
  uint8_t boost_factor = beatsin8 (6, 0, 255);
  uint8_t bonus_spawn = beatsin8 (4, 0, 17);
  firefarm->update (boost_factor, bonus_spawn);
}

void run_zigzag () {
  blender_b->blend_palette ();
  EVERY_N_SECONDS (PALETTE_DURATION_S * 0.69) {
    blender_b->next ();
  }
  EVERY_N_SECONDS (ZIGZAG_SPAWN_INTERVAL_S) {
    if (random8 (100) < ZIGZAG_SPAWN_LIKELIHOOD) {
      zigzagfarm->spawn ();
    }
  }
  zigzagfarm->draw ();
}

void run_streaks () {
  blender_c->blend_palette ();
  EVERY_N_SECONDS (PALETTE_DURATION_S * 0.77) {
    blender_c->next ();
  }
  EVERY_N_SECONDS (STREAK_INTERVAL_S * 2) {
    streaks->start_counter (STREAK_INTERVAL_S);
  }
  streaks->advance_palette ();
  streaks->update_streaks ();  
}


  
void loop() {
  timer->start ();
  

  run_firefarm ();
  //run_zigzag ();
  run_streaks ();

  blur_amount = beatsin8 (10, 8, 128, 42);
  blur1d (rgbarray, ALL_LEDS, blur_amount);


  fadeToBlackBy (rgbarray, ALL_LEDS, FADE_RATE);
  

  FastLED.show ();
  
  FastLED.delay (1000/FRAMES_PER_SECOND);
  
  timer->stop ();
   


}
