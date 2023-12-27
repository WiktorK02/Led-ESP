#include <FastLED.h>

#define LED_PIN     16 // Define the pin to which your LEDs are connected
#define NUM_LEDS    50  // Define the number of LEDs in your setup
#define BRIGHTNESS  200  // Adjust the overall brightness
#define SMOOTHNESS  20   // Adjust the smoothness of the effect

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

// Function to generate a smooth flicker pattern for each LED
void generateFlickerPattern(int flickerPatterns[NUM_LEDS][60]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int j = 0; j < 60; j++) {
      flickerPatterns[i][j] = random(40, 100);  // Adjust the range based on your preferences
    }
  }
}

// Function to update the LED colors based on the flicker patterns
void updateLedColors(int flickerPatterns[NUM_LEDS][60]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    int flickerIndex = random(0, 60);
    int targetBrightness = flickerPatterns[i][flickerIndex];

    // Smoothly transition between current brightness and target brightness using a sinusoidal function
    for (int b = leds[i].r; b != targetBrightness; b += (targetBrightness > b) ? 1 : -1) {
      float factor = sin((b - leds[i].r) * PI / (2 * SMOOTHNESS));
      leds[i].r = b;
      leds[i].g = (165 * b) / 255 * factor;  // Adjust values for orange color
      FastLED.show();
      FastLED.delay(5); // Adjust the delay for smoother transition
    }
  }
}

void loop() {
  static int flickerPatterns[NUM_LEDS][60];
  generateFlickerPattern(flickerPatterns);

  while (true) {
    updateLedColors(flickerPatterns);

    // Adjust the delay to control the speed of the flickering effect
    delay(100);
  }
}
