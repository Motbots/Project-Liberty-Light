/* ------------------------------------------------------------------------------
 * Project: Project Liberty Light: The Living Flag Project -
 * Part 5: Audio Control and Media Management
 *
 * File: Project_Liberty_Light_Part_5.ino
 * Written by: Dustin Hodges (Motbots)
 * Date Created: 06/06/2026
 * Date Last Modified: 06/06/2026
 * Description: This is Part 5 of Project Liberty Light, a multi-part build where
 * we combine electronics, embedded systems, and a bit of creative storytelling to
 * create something truly unique: a living, interactive American flag.
 *
 * Microcontroller Board: Arduino Uno R3
 * IDE Version: Arduino IDE 2.3.9
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
 * -  2x Momentary tactile push-button switches
 *
 * Article URL: https://motbots.com/project-liberty-light-part-5/
 *
 * Star-Spangled Banner Audio File:
 * https://archive.org/details/national-anthem-united-states-star-spangled-banner 
 *
 * America the Beautiful Audio File:
 * https://archive.org/details/PatrioticMedleyFeaturingTheMilitaryBands/100010365.mp3
 *
 * God Bless America Audio File:
 * https://archive.org/details/78_god-bless-america_peter-hanley-the-sandpiper-chorus-irving-berlin-mitchell-miller_gbia0534621
 *
 * Yankee Doodle Audio File:
 * https://archive.org/details/lp_the-spirit-of-76-music-for-fifes-and-drums_eastman-wind-ensemble/disc1/01.01.+Yankee+Doodle%3B+Sergeant+O'Leary%3B+The+Belle+Of+The+Mohawk+Vale.mp3
 *
 * Rally 'Round the Flag Audio File:
 * https://archive.org/details/lp_the-spirit-of-76-music-for-fifes-and-drums_eastman-wind-ensemble/disc1/02.04.+Rally+'Round+The+Flag%3B+Bonnie+Blue+Flag%3B+White+Cockade.mp3
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

#define PLAY_PAUSE_BUTTON_PIN 8 // Play/Pause button
#define NEXT_TRACK_BUTTON_PIN 9 // Next Track button

const int RX = 5;             // This is what we'll designate as the RX pin on the Arduino (not the DF0768's RX pin)
const int TX = 4;             // This is what we'll designate as the TX pin on the Arduino (not the DF0768's TX pin)

CRGB leds[NUM_LEDS];

const int flagButtonPin  = 2;
const int colorButtonPin = 3;

bool lastPlayPauseButtonState = HIGH;
bool lastNextTrackButtonState = HIGH;

bool dfPlayerReady = false;
bool audioPlaying = false;      // Initially want audio to be off when system powered on

int currentTrack = 1;           // Initial track state
const int totalTracks = 5;      // Total number of tracks available

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
  // Serial setup
  Serial.begin(115200); // Want to be able to use Serial Monitor for debugging purposes.
  delay(1000);
  Serial.println("PART 5 SKETCH STARTED");

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Pin modes
  pinMode(flagButtonPin, INPUT_PULLUP);
  pinMode(colorButtonPin, INPUT_PULLUP);
  pinMode(PLAY_PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_TRACK_BUTTON_PIN, INPUT_PULLUP);

  showCurrentMode();    // Turn LEDs on first

  soundFxSerial.begin(115200);
  delay(2000);

  dfPlayerReady = soundFxPlayer.begin(soundFxSerial);

  if (dfPlayerReady) {
    Serial.println("DFPlayer Pro online.");

    soundFxPlayer.setPrompt(false);
    soundFxPlayer.switchFunction(soundFxPlayer.MUSIC);
    soundFxPlayer.setVol(10);
  } 
  else {
    Serial.println("DFPlayer Pro not detected. Continuing without audio.");
  }
}

void loop() {
  playState();
  nextTrackState();
  handleFlagButton();
  handleColorButton();
}

void playState() {
  bool currentState = digitalRead(PLAY_PAUSE_BUTTON_PIN);

  if (lastPlayPauseButtonState == HIGH && currentState == LOW) {
    audioPlaying = !audioPlaying;

    if (audioPlaying && dfPlayerReady) {
      soundFxPlayer.playFileNum(currentTrack);
      Serial.print("Playing track: ");
      Serial.println(currentTrack);
    } 
    else if (dfPlayerReady) {
      soundFxPlayer.pause();
      Serial.println("Audio paused.");
    }

    delay(200); // simple debouce
  }

  lastPlayPauseButtonState = currentState;
}

void nextTrackState() {
  bool currentState = digitalRead(NEXT_TRACK_BUTTON_PIN);

  if (lastNextTrackButtonState == HIGH && currentState == LOW) {
    currentTrack++;

    if (currentTrack > totalTracks) {
      currentTrack = 1;
    }

    Serial.print("Next track selected: ");
    Serial.println(currentTrack);

    if (audioPlaying && dfPlayerReady) {
      soundFxPlayer.playFileNum(currentTrack);
    }

    delay(200); // simple debouce
  }

  lastNextTrackButtonState = currentState;
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