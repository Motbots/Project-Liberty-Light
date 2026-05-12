/* ------------------------------------------------------------------------------
 * Project: Project Liberty Light: The Living Flag Project -
 * Part 1: Lighting the Foundation
 *
 * File: Project_Liberty_Light_Part_1.ino
 * Written by: Dustin Hodges (Motbots)
 * Date Created: 04/20/2026
 * Date Last Modified: 05/11/2026
 * Description: This is Part 1 of Project Liberty Light, a multi-part build where
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
 *
 * Article URL: https://motbots.com/project-liberty-light-part-1/
 * ------------------------------------------------------------------------------
 */

#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 128
#define BRIGHTNESS 40

#define CANTON_X_START 2
#define CANTON_X_END   8
#define CANTON_HEIGHT  4

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

// Get the exact LED location number based off the 16x2 matrix
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

void loop() {

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {

      int i = getIndex(x, y);

      // Stripes
      if (y % 2 == 0) {
        leds[i] = CRGB::Red;
      } else {
        leds[i] = CRGB::White;
      }

      // Blue canton (top-left corner)
      if (x >= CANTON_X_START && x < CANTON_X_END && y < CANTON_HEIGHT) {
        leds[i] = CRGB::Blue;
      }
    }
  }

  FastLED.show();
}