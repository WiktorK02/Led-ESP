#include <FastLED.h>

FASTLED_USING_NAMESPACE

const int pwmInputPin = 27;
volatile unsigned long pulseWidth = 0;
unsigned long MIN_PULSE_WIDTH_MICROS = 1;
unsigned long MAX_PULSE_WIDTH_MICROS = 4080;

#define DATA_PIN    25
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    50
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

int reversedValue = 0;
int filteredReversedValue = 0;
const int filterWindowSize = 5;
int filterValues[filterWindowSize];
int filterIndex = 0;

void setup() {
    Serial.begin(115200);
    pinMode(pwmInputPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pwmInputPin), handleInterrupt, CHANGE);

    delay(3000);
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
}

typedef void (*SimplePatternList)();
SimplePatternList gPatterns[] = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };
uint8_t gCurrentPatternNumber = 0;
uint8_t gHue = 0;

void loop() {
    static unsigned long lastResetTime = 0;
    int mappedValue = map(pulseWidth, MIN_PULSE_WIDTH_MICROS, MAX_PULSE_WIDTH_MICROS, 0, 255);
    mappedValue = constrain(mappedValue, 0, 255);
    reversedValue = 255 - mappedValue;

    // Update filter values
    filterValues[filterIndex] = reversedValue;
    filterIndex = (filterIndex + 1) % filterWindowSize;

    // Sort filter values
    int sortedValues[filterWindowSize];
    memcpy(sortedValues, filterValues, sizeof(sortedValues));
    std::sort(sortedValues, sortedValues + filterWindowSize);

    // Choose the median value
    filteredReversedValue = sortedValues[filterWindowSize / 2];

    static int prevFilteredValue = -1;

    if (filteredReversedValue != prevFilteredValue) {
        Serial.print("Value: ");
        Serial.println(filteredReversedValue);
        prevFilteredValue = filteredReversedValue;

        nextPattern(); 
    }

    if (millis() - lastResetTime >= 1000) {
        lastResetTime = millis();
        filteredReversedValue = 0;
    }

    gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    FastLED.delay(1000 / FRAMES_PER_SECOND);
    EVERY_N_MILLISECONDS(20) { gHue++; }
    EVERY_N_SECONDS(10) { nextPattern(); }
}

void handleInterrupt() {
    static unsigned long startTime = 0;
    if (digitalRead(pwmInputPin) == HIGH) {
        startTime = micros();
    } else {
        pulseWidth = micros() - startTime;
    }
}

void nextPattern() {
    if (filteredReversedValue == 255) {
        gCurrentPatternNumber = (gCurrentPatternNumber + 1) % (sizeof(gPatterns) / sizeof(gPatterns[0]));
    }
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{

  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
