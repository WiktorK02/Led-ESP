#include <FastLED.h>

#define NUM_LEDS  50
#define LED_PIN   16

CRGB leds[NUM_LEDS];
uint8_t colorIndex[NUM_LEDS];
uint8_t darkenRate[NUM_LEDS];

DEFINE_GRADIENT_PALETTE( fire_gp ) { 
  0,   0,  0,  0,   // Black
  128, 255, 0,  0,   // Red
  192, 255, 69, 0,   // Red-Orange
  255, 255, 255, 0   // Yellow
};

CRGBPalette16 fire = fire_gp;

void setup() {
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(200);

  // Fill the colorIndex array with random numbers
  for (int i = 0; i < NUM_LEDS; i++) {
    colorIndex[i] = random8();
    darkenRate[i] = random8(1, 5);  // Random darken rate between 1 and 4
  }
}

void loop() {
  // Color each pixel from the palette using the index from colorIndex[]
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(fire, colorIndex[i], 255); // Max brightness
  }

  // Darken each LED individually at a random rate
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].fadeToBlackBy(darkenRate[i]);
    colorIndex[i]++;
  }

  FastLED.show();
  delay(10);  // Adjust the delay if needed
}
