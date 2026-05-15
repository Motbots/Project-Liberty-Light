/* ------------------------------------------------------------------------------
 * Project: Project Liberty Light: The Living Flag Project -
 * Part 2: Adding Button Control
 *
 * File: Project_Liberty_Light_Part_2.ino
 * Written by: Dustin Hodges (Motbots)
 * Date Created: 04/21/2026
 * Date Last Modified: 05/13/2026
 * Description: This is Part 2 of Project Liberty Light, a multi-part build where
 * we combine electronics, embedded systems, and a bit of creative storytelling to
 * create something truly unique: a living, interactive American flag.
 *
 * Microcontroller Board: Arduino Uno R3
 * IDE Version: Arduino IDE 2.3.8
 * Items Used:
 * -  Arduino Uno
 * -  2x WS2812B 8x8 LED matrices
 * -  Breadboard
 * -  330Ω resistor
 * -  1000µF capacitor
 * -  Jumper wires
 * -  2x Momentary push-button switches
 *
 * Article URL: https://motbots.com/project-liberty-light-part-2/
 * ------------------------------------------------------------------------------
 */

#include <FastLED.h>

#define LED_PIN      6
#define NUM_LEDS     128
#define CHIPSET      WS2812B
#define COLOR_ORDER  GRB
#define BRIGHTNESS   40

#define CANTON_X_START 2
#define CANTON_X_END   8
#define CANTON_HEIGHT  4

CRGB leds[NUM_LEDS];

const int flagButtonPin  = 2;
const int colorButtonPin = 3;

enum DisplayMode {
  MODE_FLAG,
  MODE_RED,
  MODE_WHITE,
  MODE_BLUE,
  MODE_OFF
};

DisplayMode currentMode = MODE_FLAG;

// Button state tracking
bool lastFlagButtonState = HIGH;
bool lastColorButtonState = HIGH;

void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  pinMode(flagButtonPin, INPUT_PULLUP);
  pinMode(colorButtonPin, INPUT_PULLUP);

  showCurrentMode();
}

void loop() {
  handleFlagButton();
  handleColorButton();
}

// Same getIndex() we used from Part 1
int getIndex(int x, int y) {
  if (x < 8) {
    // Left matrix
    if (y % 2 == 0) {
      return y * 8 + x;
    } else {
      return y * 8 + (7 - x);
    }
  } else {
    // Right matrix
    int x2 = x - 8;
    if (y % 2 == 0) {
      return 64 + y * 8 + x2;
    } else {
      return 64 + y * 8 + (7 - x2);
    }
  }
}

void handleFlagButton() {
  bool currentState = digitalRead(flagButtonPin);

  if (lastFlagButtonState == HIGH && currentState == LOW) {
    currentMode = MODE_FLAG;
    showCurrentMode();
    delay(200);  // simple debounce
  }

  lastFlagButtonState = currentState;
}

void handleColorButton() {
  bool currentState = digitalRead(colorButtonPin);

  if (lastColorButtonState == HIGH && currentState == LOW) {
    cycleColorMode();
    showCurrentMode();
    delay(200);  // simple debounce
  }

  lastColorButtonState = currentState;
}

void cycleColorMode() {
  if (currentMode == MODE_FLAG) {
    currentMode = MODE_RED;
  } 
  else if (currentMode == MODE_RED) {
    currentMode = MODE_WHITE;
  } 
  else if (currentMode == MODE_WHITE) {
    currentMode = MODE_BLUE;
  } 
  else if (currentMode == MODE_BLUE) {
    currentMode = MODE_OFF;
  } 
  else {
    currentMode = MODE_RED;
  }
}

void showCurrentMode() {
  switch (currentMode) {
    case MODE_FLAG:
      drawAmericanFlag();
      break;

    case MODE_RED:
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.show();
      break;

    case MODE_WHITE:
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.show();
      break;

    case MODE_BLUE:
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.show();
      break;

    case MODE_OFF:
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      break;
  }
}

void drawAmericanFlag() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Existing flag code from Part 1
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      int i = getIndex(x, y);

      // Stripes
      if (y % 2 == 0) {
        leds[i] = CRGB::Red;
      } else {
        leds[i] = CRGB::White;
      }

      // Blue canton
      if (x >= CANTON_X_START && x < CANTON_X_END && y < CANTON_HEIGHT) {
        leds[i] = CRGB::Blue;
      }
    }
  }

  FastLED.show();
}