 #include <FastLED.h>
#include <Bounce2.h>

#define LED_PIN     16
#define COLOR_ORDER RGB
#define CHIPSET     WS2811
#define NUM_LEDS    100
#define BRIGHTNESS  20
#define FRAMES_PER_SECOND 120
#define COOLING  150
#define SPARKING 200
#define BRIGHTNESS_INCREMENT 50
#define BRIGHTNESS_CHANGE_FACTOR 5

int targetBrightness = BRIGHTNESS;
int currentBrightness = BRIGHTNESS;
int switchPin1 = 2; // Switch for brightness control
int switchPin2 = 18; // Switch for color change
int val;
int buttonState1;
int buttonState2;
int buttonPresses = 0;
bool gReverseDirection = false;

CRGB leds[NUM_LEDS];
CRGBPalette16 palettes[] = {
  CRGBPalette16(CRGB::Red, CRGB::Orange),
  CRGBPalette16(CRGB::Blue, CRGB::Red)
};
int currentPaletteIndex = 0; // Index of the current palette
Bounce debouncer1 = Bounce();
Bounce debouncer2 = Bounce();

void setup() {
  delay(3000);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(switchPin1, INPUT_PULLUP);
  pinMode(switchPin2, INPUT_PULLUP);
  Serial.begin(9600);
  buttonState1 = digitalRead(switchPin1);
  buttonState2 = digitalRead(switchPin2);
  debouncer1.attach(switchPin1);
  debouncer1.interval(50);
  debouncer2.attach(switchPin2);
  debouncer2.interval(50);
}

void loop() {
  random16_add_entropy(random());
  Fire2012WithPalette();

  debouncer1.update();
  if (debouncer1.fell()) {
    buttonPresses++;
    targetBrightness = BRIGHTNESS + (buttonPresses * BRIGHTNESS_INCREMENT);
    if (targetBrightness > 255) {
      targetBrightness = BRIGHTNESS;
      buttonPresses = 0;
    }
  }

  debouncer2.update();
  if (debouncer2.fell()) {
    // Change color palette when the second switch is pressed
    currentPaletteIndex = (currentPaletteIndex + 1) % (sizeof(palettes) / sizeof(palettes[0]));
  }

  int brightnessDifference = abs(currentBrightness - targetBrightness);
  int brightnessChange = max(1, brightnessDifference / BRIGHTNESS_CHANGE_FACTOR);

  if (currentBrightness < targetBrightness) {
    currentBrightness = min(targetBrightness, currentBrightness + brightnessChange);
  } else if (currentBrightness > targetBrightness) {
    currentBrightness = max(targetBrightness, currentBrightness - brightnessChange);
  }

  FastLED.setBrightness(currentBrightness);
  FastLED.show();
  FastLED.delay(3000 / FRAMES_PER_SECOND);
}

void Fire2012WithPalette() {
  static byte heat[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  for (int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  if (random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(100, 255));
  }

  for (int j = 0; j < NUM_LEDS; j++) {
    byte colorindex = scale8(heat[j], 128);
    leds[j] = ColorFromPalette(palettes[currentPaletteIndex], colorindex);
  }
}

