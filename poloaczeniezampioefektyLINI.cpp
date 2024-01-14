#include <FastLED.h>

const int pwmInputPin = 27;
volatile unsigned long pulseWidth = 0;
unsigned long MIN_PULSE_WIDTH_MICROS = 1;
unsigned long MAX_PULSE_WIDTH_MICROS = 4080;

#define DATA_PIN    25
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    50
CRGB leds[NUM_LEDS];

int brightness = 255;
#define FRAMES_PER_SECOND  120

int reversedValue = 0;
int filteredReversedValue = 0;
const int filterWindowSize = 5;
int filterValues[filterWindowSize];
int filterIndex = 0;
bool is255;
bool is254 = true;

void setup() {
    Serial.begin(115200);
    pinMode(pwmInputPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pwmInputPin), handleInterrupt, CHANGE);

    delay(3000);
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(brightness);
}

typedef void (*SimplePatternList)();
SimplePatternList gPatterns[] = {  
    singleLineMoveRight, singleLineMoveLeft, 
    lineFillsEntireStrip, lineFillsEntireStripReversed, linesMeetInCenterAndStay,
    linesMeetInCenterAndStay2, linesMeetAtCenter
};
uint8_t gCurrentPatternNumber = 0;
uint8_t gHue = 0;

void loop() {
    static unsigned long lastResetTime = 0;
    static int filteredCounter = 1;  
    int mappedValue = map(pulseWidth, MIN_PULSE_WIDTH_MICROS, MAX_PULSE_WIDTH_MICROS, 0, 255);
    mappedValue = constrain(mappedValue, 0, 255);
    reversedValue = 255 - mappedValue;

    filterValues[filterIndex] = reversedValue;
    filterIndex = (filterIndex + 1) % filterWindowSize;

    int sortedValues[filterWindowSize];
    memcpy(sortedValues, filterValues, sizeof(sortedValues));
    std::sort(sortedValues, sortedValues + filterWindowSize);

    filteredReversedValue = sortedValues[filterWindowSize / 2];

    static int prevFilteredValue = -1;

    if (filteredReversedValue != prevFilteredValue) {
        Serial.print("Value: ");
        Serial.println(filteredReversedValue);
        prevFilteredValue = filteredReversedValue;

        if (filteredReversedValue == 255) {
            is255 = true;
            is254 = false;
            FastLED.setBrightness(255);
            nextPattern();
            filteredCounter = 1; 
        }
        else if (filteredReversedValue == 254){
            if (filteredCounter % 2 == 0) {
                fill_solid(leds, NUM_LEDS, CRGB::Black);  
                FastLED.setBrightness(0);
                is255 = false;
                is254 = false;
            } else {
                fill_solid(leds, NUM_LEDS, CRGB::Blue);
                FastLED.setBrightness(brightness);
                is255 = false;
                is254 = true;
            }
            filteredCounter++;
        }
        else if (filteredReversedValue == 253 && !is255 ){
            brightness = brightness - 50;
            FastLED.setBrightness(brightness);
        }
    }

    if(is255) gPatterns[gCurrentPatternNumber]();
    FastLED.show();
    FastLED.delay(2500 / FRAMES_PER_SECOND);
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

void singleLineMoveRight() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    static int pos = 0;
    leds[pos] = CRGB::White;
    pos = (pos + 1) % NUM_LEDS;
    FastLED.show();

}

void singleLineMoveLeft() {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    static int pos = NUM_LEDS - 1;
    leds[pos] = CRGB::White;
    pos = (pos - 1 + NUM_LEDS) % NUM_LEDS;
    FastLED.show();

}


void lineFillsEntireStrip() {
  static int pos = 0;
  
  // Check if reset is requested
  if (filteredReversedValue == 255) {
    pos = 0; // Reset position to 0
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (int i = 0; i <= pos; ++i) {
    leds[i] = CRGB::White;
  }

  FastLED.show();

  if (pos < NUM_LEDS - 1) {
    pos++;
  }
}
void lineFillsEntireStripReversed() {
  static int pos = NUM_LEDS - 1;  // Start from the end of the strip
  
  // Check if reset is requested
  if (filteredReversedValue == 255) {
    pos = NUM_LEDS - 1; // Reset position to the end
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);

  for (int i = 0; i <= pos-1; ++i) {
    leds[i] = CRGB::White;
  }

  FastLED.show();

  if (pos > 0) {
    pos--;
  }
}

void linesMeetInCenterAndStay() {
    static int leftPos = 0;
    static int rightPos = NUM_LEDS - 1;
  
    // Check if reset is requested
    if (filteredReversedValue == 255) {
        leftPos = 0;   // Reset left position to 0
        rightPos = NUM_LEDS - 1;  // Reset right position to the end
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);

    leds[leftPos] = CRGB::White;
    leds[rightPos] = CRGB::White;

    FastLED.show();

    if (leftPos < rightPos) {
        leftPos++;
        rightPos--;
    }
}
void linesMeetInCenterAndStay2() {
    static int topPos = 0;
    static int bottomPos = NUM_LEDS - 1;
  
    // Check if reset is requested
    if (filteredReversedValue == 255) {
        topPos = 0;      // Reset top position to 0
        bottomPos = NUM_LEDS - 1;  // Reset bottom position to the end
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Draw the line from topPos to bottomPos
    for (int i = topPos; i <= bottomPos; ++i) {
        leds[i] = CRGB::White;
    }

    FastLED.show();

    if (topPos < bottomPos) {
        topPos++;
        bottomPos--;
    }
}

void linesMeetAtCenter() {
    static int leftPos = 0;
    static int rightPos = NUM_LEDS - 1;
    static bool linesMeeting = false;

    // Check if reset is requested
    if (filteredReversedValue == 255) {
        leftPos = 0;   // Reset left position to 0
        rightPos = NUM_LEDS - 1;  // Reset right position to the end
        linesMeeting = false;
    }

    fill_solid(leds, NUM_LEDS, CRGB::Black);

    if (!linesMeeting) {
        // Draw the line moving from the left
        for (int i = 0; i <= leftPos; ++i) {
            leds[i] = CRGB::White;
        }

        // Draw the line moving from the right
        for (int i = NUM_LEDS - 1; i >= rightPos; --i) {
            leds[i] = CRGB::White;
        }

        // Move the lines towards the center
        if (leftPos < rightPos) {
            leftPos++;
            rightPos--;
        } else {
            linesMeeting = true;  // Lines have met at the center
        }
    } else {
        // Lines have met, fill the entire strip with white color
        fill_solid(leds, NUM_LEDS, CRGB::White);
    }

    FastLED.show();
}

