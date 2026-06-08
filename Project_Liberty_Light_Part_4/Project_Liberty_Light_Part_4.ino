/* ------------------------------------------------------------------------------
 * Project: Project Liberty Light: The Living Flag Project -
 * Part 4: Audio Integration
 *
 * File: Project_Liberty_Light_Part_4.ino
 * Written by: Dustin Hodges (Motbots)
 * Date Created: 05/25/2026
 * Date Last Modified: 05/25/2026
 * Description: This is Part 4 of Project Liberty Light, a multi-part build where
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
 * -  DFPlayer Pro Mini MP3 Player
 * -  2x 3W 8Ω Mini Speakers
 *
 * Article URL: https://motbots.com/project-liberty-light-part-4/
 *
 * Star-Spangled Banner Audio File:
 * https://archive.org/details/national-anthem-united-states-star-spangled-banner 
 *
 * ------------------------------------------------------------------------------
 */

#include <FastLED.h>          // The FastLED library for using the WS2812B 8x8 LED matrices
#include <DFRobot_DF1201S.h>  // The DFRobot_DF1201S library for the DFPlayer Pro Mini MP3 Player
#include <SoftwareSerial.h>   // The SoftwareSerial library allows serial communication on
                              // other digital pins of an Arduino board.

#define LED_PIN      6
#define NUM_LEDS     128
#define CHIPSET      WS2812B
#define COLOR_ORDER  GRB
#define BRIGHTNESS   40

#define CANTON_X_START 2
#define CANTON_X_END   8
#define CANTON_HEIGHT  4

const int RX = 5;             // This is what we'll designate as the RX pin on the Arduino (not the DF0768's RX pin)
const int TX = 4;             // This is what we'll designate as the TX pin on the Arduino (not the DF0768's TX pin)

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

SoftwareSerial soundFxSerial(RX, TX);   // Creating an object for our sound effects serial, saying which pins to use
                                        // for receiving (RX) and transmission (TX).
                                        // The SoftwareSerial is used to communicate with the DFPlayer Pro module.

DFRobot_DF1201S soundFxPlayer;          // We'll use this object when referring to the sound effects player (MP3 player).

void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Pin modes
  pinMode(flagButtonPin, INPUT_PULLUP);
  pinMode(colorButtonPin, INPUT_PULLUP);
  pinMode(RX, INPUT);   // Our receiving pin for the Arduino needs to be an input.
  pinMode(TX, OUTPUT);  // Our transmission pin for the Arduino needs to be an output.

  // Serial setup
  Serial.begin(115200);               	// Want to be able to use Serial Monitor for debugging purposes.
  soundFxSerial.begin(115200);          // Setting up the serial for the sound effects serial.
  soundFxPlayer.begin(soundFxSerial);   // Telling the sound effects player to use the sound effects serial.

  while (!soundFxPlayer.begin(soundFxSerial)) {
    Serial.println("DFPlayer Pro not detected.");
    delay(1000);
  }

  Serial.println("DFPlayer Pro online.");

  // Player setup
  soundFxPlayer.setVol(30);                           // Set volume for the sound effects player. The volume range is from 0 to 30.
  soundFxPlayer.switchFunction(soundFxPlayer.MUSIC);  // Enter music mode.
  soundFxPlayer.setPrompt(false);                     // Silence voice prompt at start.
  delay(2000);                                        // Delay for two seconds to allow everything to calibrate
                                                      // before proceeding.

  soundFxPlayer.playFileNum(1);		                    // Play the first file

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