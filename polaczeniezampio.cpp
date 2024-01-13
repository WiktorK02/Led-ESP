#include <FastLED.h>

#define LED_PIN           25
#define NUM_LEDS          50
#define INITIAL_BRIGHTNESS 255
#define MAX_BRIGHTNESS    255
#define MIN_BRIGHTNESS    0
#define FADE_SPEED        4
#define PALETTE_INTERVAL  5000  // Czas przed zmniejszeniem jasności (w milisekundach)
#define FADE_DELAY        10000  // Dodatkowe opóźnienie przed rozpoczęciem zanikania jasności
#define FRAMES_PER_SECOND 120

CRGB leds[NUM_LEDS];

DEFINE_GRADIENT_PALETTE(blue_palette) {
  0,    0,   0, 255,   // Ciemnoniebieski
  32,   0,  32, 255,   // Niebieski
  64,   0,  64, 255,   // Średnio niebieski
  96,   0,  96, 255,   // Jasnoniebieski
  255,   0, 0, 255      // Ciemniejszy niebieski10
};

DEFINE_GRADIENT_PALETTE(purple_palette) {
  0,    255,   0, 0,   // Ciemnoniebieski
  32,   255,  16, 0,   // Niebieski
  64,   255,  32, 0,   // Średnio niebieski
  64,   255,  48, 0,   // Średnio niebieski
  255,   255, 0, 0      // Ciemniejszy niebieski10
};

DEFINE_GRADIENT_PALETTE(orange_palette) {
  0,    0,   255, 0,   // Ciemnoniebieski
  32,   0,  255, 16,   // Niebieski
  64,   0,  255, 32,   // Średnio niebieski
  96,   0,  255, 48,   // Jasnoniebieski
  255,   0, 255, 0      // Ciemniejszy niebieski10
};
DEFINE_GRADIENT_PALETTE(green_palette) {
  0,    0,   255, 0,   // Ciemnoniebieski
  32,   16,  255, 0,   // Niebieski
  64,   32,  255, 0,   // Średnio niebieski
  96,   48,  255, 0,   // Jasnoniebieski
  255,   0, 255, 0      // Ciemniejszy niebieski10
};

CRGBPalette16 palettes[] = {blue_palette, purple_palette, orange_palette, green_palette};
int currentPalette = 0;
uint8_t currentBrightness = INITIAL_BRIGHTNESS;
bool transition_enable = false; // wazne 

const int pwmInputPin = 27; // Change this to the GPIO pin you are using
volatile unsigned long pulseWidth = 0;
unsigned long MIN_PULSE_WIDTH_MICROS = 1; // Adjust as needed
unsigned long MAX_PULSE_WIDTH_MICROS = 4080; // Adjust as needed

const unsigned long debounceDelay = 100;  // Increase debounce delay
unsigned long lastDebounceTime = 0;
int lastMappedValue = 0;

void setup() {
  FastLED.addLeds<WS2811, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);
  Serial.begin(115200);
  pinMode(pwmInputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pwmInputPin), handleInterrupt, CHANGE);
}

void loop() {
  if (transition_enable) fadeToMaxBrightness();
  plasma();
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  static unsigned long lastResetTime = 0;

  int mappedValue = map(pulseWidth, MIN_PULSE_WIDTH_MICROS, MAX_PULSE_WIDTH_MICROS, 0, 255);
  mappedValue = constrain(mappedValue, 0, 255);

  // Debouncing
  if (abs(mappedValue - lastMappedValue) > 10) {  // Adjust the threshold
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    int reversedValue = 255 - mappedValue;

    static int prevReversedValue = -1; // Initialize to a value that won't match any valid reversed value

    if (reversedValue != prevReversedValue) {
      Serial.print("Value: ");
      Serial.println(reversedValue);

      // Check if reversedValue is 255
      if (reversedValue == 255) {
        // Change color palette or perform any other actions
        currentPalette = (currentPalette + 1) % (sizeof(palettes) / sizeof(palettes[0]));
      }

      prevReversedValue = reversedValue;
    }
  }

  // Check if one second has passed
  if (millis() - lastResetTime >= 1000) {
    lastResetTime = millis();
    lastMappedValue = 0; // Reset lastMappedValue to 0 after one second
  } else {
    lastMappedValue = mappedValue;
  }
}

void fadeToMaxBrightness() {
  static uint32_t lastFadeTime = 0;
  static bool fadingIn = true;
  static uint32_t fadeStartTime = 0;

  if (fadingIn) {
    if (currentBrightness < MAX_BRIGHTNESS) {
      currentBrightness = min(currentBrightness + FADE_SPEED, MAX_BRIGHTNESS);
    } else {
      if (millis() - fadeStartTime >= FADE_DELAY) {
        fadingIn = false;
        fadeStartTime = millis();
      }
    }
  } else {
    if (currentBrightness > MIN_BRIGHTNESS) {
      currentBrightness = max(currentBrightness - FADE_SPEED, MIN_BRIGHTNESS);
    } else {
      fadingIn = true;
      if (millis() - lastFadeTime >= PALETTE_INTERVAL) {
        lastFadeTime = millis();
        currentPalette = (currentPalette + 1) % (sizeof(palettes) / sizeof(palettes[0]));
      }
    }
  }

  FastLED.setBrightness(currentBrightness);
}

void plasma() {
  static uint8_t time = 0;

  for (int i = 0; i < NUM_LEDS; i++) {
    uint8_t v = sin8(i * 5 + time) + cos8(i * 3 + time) + sin8(i * 7 + time);
    CRGB color = ColorFromPalette(palettes[currentPalette], (v + time) % 255, 255, LINEARBLEND);
    leds[i] = color;
  }

  time++;
}

void handleInterrupt() {
  static unsigned long startTime = 0;

  if (digitalRead(pwmInputPin) == HIGH) {
    startTime = micros();
  } else {
    pulseWidth = micros() - startTime;
  }
}
