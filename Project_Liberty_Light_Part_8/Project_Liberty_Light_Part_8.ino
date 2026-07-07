/* ------------------------------------------------------------------------------
 * Project: Project Liberty Light: The Living Flag Project -
 * Part 8: Status Indicators and Audio Volume Control
 *
 * File: Project_Liberty_Light_Part_8.ino
 * Written by: Dustin Hodges (Motbots)
 * Date Created: 07/05/2026
 * Date Last Modified: 07/07/2026
 * Description: This is Part 8 of Project Liberty Light, a multi-part build where
 * we combine electronics, embedded systems, and a bit of creative storytelling to
 * create something truly unique: a living, interactive American flag.
 *
 * Microcontroller Board: Arduino Uno R3
 * IDE Version: Arduino IDE 2.3.10
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
 * -  3x Indicator LEDs
 * -  3x 220Ω Resistors
 * -  10kΩ Potentiometer
 *
 * Article URL: https://motbots.com/project-liberty-light-part-8/
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

const int RX = 5;               // This is what we'll designate as the RX pin on the Arduino (not the DF0768's RX pin)
const int TX = 4;               // This is what we'll designate as the TX pin on the Arduino (not the DF0768's TX pin)

CRGB leds[NUM_LEDS];

const int flagButtonPin  = 2;
const int celebrationButtonPin = 3;

// Defining the pins for the indicator LEDs
const int powerLED = 10;
const int flagLED = 11;
const int celebrationLED = 12;

// Defining the pin for the potentiometer
const int volumePot = A0;

bool lastPlayPauseButtonState = HIGH;
bool lastNextTrackButtonState = HIGH;

bool dfPlayerReady = false;
bool audioPlaying = false;      // Initially want audio to be off when system powered on

int currentTrack = 1;           // Initial track state
const int totalTracks = 5;      // Total number of tracks available

enum DisplayMode {
  MODE_STATIC_FLAG,
  MODE_WAVING_FLAG,
  MODE_TWINKLE_FLAG,
  MODE_PATRIOTIC_PULSE
};

enum CelebrationMode {
  CELEBRATION_OFF,
  CELEBRATION_FIREWORKS,
  CELEBRATION_SEQUENCE
};

DisplayMode currentMode = MODE_STATIC_FLAG;
CelebrationMode celebrationMode = CELEBRATION_OFF;

// Adding animation timing
unsigned long previousAnimationTime = 0;
const unsigned long animationInterval = 50;

// Adding Celebration Timing Variables
unsigned long celebrationTimer = 0;
byte celebrationStage = 0;

// waveOffset keeps track of the wave's current position
uint8_t waveOffset = 0;

// pulseOffset controls current position of the pulse animation
uint8_t pulseOffset = 0;

// Button state tracking
bool lastFlagButtonState = HIGH;
bool lastCelebrationButtonState = HIGH;

SoftwareSerial soundFxSerial(RX, TX);   // Creating an object for our sound effects serial, saying which pins to use
                                        // for receiving (RX) and transmission (TX).
                                        // The SoftwareSerial is used to communicate with the DFPlayer Pro module.

DFRobot_DF1201S soundFxPlayer;          // We'll use this object when referring to the sound effects player (MP3 player).

void setup() {
  // Serial setup
  Serial.begin(115200); // Want to be able to use Serial Monitor for debugging purposes.
  delay(1000);
  Serial.println("PART 7 SKETCH STARTED");

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  // Pin modes
  pinMode(flagButtonPin, INPUT_PULLUP);
  pinMode(celebrationButtonPin, INPUT_PULLUP);
  pinMode(PLAY_PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_TRACK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(powerLED, OUTPUT);
  pinMode(flagLED, OUTPUT);
  pinMode(celebrationLED, OUTPUT);

  digitalWrite(powerLED, HIGH);       // Initially turn on the power indicator LED
  digitalWrite(flagLED, HIGH);        // Initially turn on the flag indicator LED
  digitalWrite(celebrationLED, LOW);  // Initially turn off the celebration indicator LED

  drawAmericanFlag(); // display the regular static flag

  soundFxSerial.begin(115200);
  delay(2000);

  dfPlayerReady = soundFxPlayer.begin(soundFxSerial);

  if (dfPlayerReady) {
    Serial.println("DFPlayer Pro online.");

    soundFxPlayer.setPrompt(false);
    soundFxPlayer.switchFunction(soundFxPlayer.MUSIC);
  } 
  else {
    Serial.println("DFPlayer Pro not detected. Continuing without audio.");
  }
}

void loop() {
  playState();
  nextTrackState();
  handleFlagButton();
  handleCelebrationButton();
  updateStatusIndicators();
  updateVolume();

  // Call Animation timing
  updateAnimationTiming();

  if (celebrationMode == CELEBRATION_FIREWORKS) {
    drawFireworks();
  }
  else if (celebrationMode == CELEBRATION_SEQUENCE) {
    runCelebrationSequence();
  }
  else {
    drawCurrentDisplayMode();
  }
}

// Organizing the Animation Timing
void updateAnimationTiming() {
  if (millis() - previousAnimationTime >= animationInterval) {
    previousAnimationTime = millis();

    waveOffset += 4;
    pulseOffset += 3;
  }
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

// Flag Button logic
void handleFlagButton() {
  bool currentState = digitalRead(flagButtonPin);

  if (lastFlagButtonState == HIGH && currentState == LOW) {

    // If Celebration Mode is active, exit it first
    if (celebrationMode != CELEBRATION_OFF) {
      celebrationMode = CELEBRATION_OFF;
      audioPlaying = false;
      currentTrack = 1;

      if (dfPlayerReady) {
        soundFxPlayer.pause();
      }

      Serial.println("Celebration Mode cancelled by Flag Button");
    }

    // Then continue normal flag mode cycling
    currentMode = (DisplayMode)((currentMode + 1) % 4);

    Serial.print("Display mode changed to: ");
    Serial.println(currentMode);

    delay(200);
  }

  lastFlagButtonState = currentState;
}

// Celebration Button logic
void handleCelebrationButton() {
  bool currentState = digitalRead(celebrationButtonPin);

  if (lastCelebrationButtonState == HIGH && currentState == LOW) {
    
    if (celebrationMode == CELEBRATION_OFF) {
      celebrationMode = CELEBRATION_FIREWORKS;

      currentTrack = 1;
      audioPlaying = true;

      if (dfPlayerReady) {
        soundFxPlayer.playFileNum(currentTrack);
      }

      Serial.println("Celebration Mode: Fireworks + Track 1");
    }

    else if (celebrationMode == CELEBRATION_FIREWORKS) {
      celebrationMode = CELEBRATION_SEQUENCE;

      celebrationStage = 0;
      celebrationTimer = millis();

      currentTrack = 1;
      audioPlaying = true;

      if (dfPlayerReady) {
        soundFxPlayer.setPlayMode(soundFxPlayer.ALLCYCLE);
        soundFxPlayer.playFileNum(currentTrack);
      }

      Serial.println("Celebration Mode: Full Sequence");
    }

    else if (celebrationMode == CELEBRATION_SEQUENCE) {
      celebrationMode = CELEBRATION_OFF;

      audioPlaying = false;
      currentTrack = 1;
      currentMode = MODE_STATIC_FLAG;

      if (dfPlayerReady) {
        soundFxPlayer.pause();
      }

      drawAmericanFlag();

      Serial.println("Celebration Mode: OFF - Returning to Static Flag");
    }

    delay(200);
  }

  lastCelebrationButtonState = currentState;
}

// Draw Current Display Mode
void drawCurrentDisplayMode() {
  switch (currentMode) {
    case MODE_STATIC_FLAG:
      drawAmericanFlag();
      break;

    case MODE_WAVING_FLAG:
      drawWavingFlag();
      break;

    case MODE_TWINKLE_FLAG:
      drawTwinklingFlag();
      break;

    case MODE_PATRIOTIC_PULSE:
      drawPatrioticPulse();
      break;
  }
}

// Run Celebratio Sequence Mode
void runCelebrationSequence() {
  if (millis() - celebrationTimer >= 10000) {
    celebrationStage++;

    if (celebrationStage > 4) {
      celebrationStage = 0;
    }

    celebrationTimer = millis();

    Serial.print("Celebration stage: ");
    Serial.println(celebrationStage);
  }

  switch (celebrationStage) {
    case 0:
      drawAmericanFlag();
      break;

    case 1:
      drawWavingFlag();
      break;

    case 2:
      drawTwinklingFlag();
      break;

    case 3:
      drawPatrioticPulse();
      break;

    case 4:
      drawFireworks();
      break;
  }
}

// Fireworks Animation function
void drawFireworks() {
  FastLED.clear();

  int centerX = random(2, 14);
  int centerY = random(1, 6);

  setPixelSafe(centerX, centerY, CRGB::White);

  setPixelSafe(centerX - 1, centerY, CRGB::Red);
  setPixelSafe(centerX + 1, centerY, CRGB::Blue);

  setPixelSafe(centerX, centerY - 1, CRGB::White);
  setPixelSafe(centerX, centerY + 1, CRGB::White);

  setPixelSafe(centerX - 1, centerY - 1, CRGB::Red);
  setPixelSafe(centerX + 1, centerY - 1, CRGB::Blue);

  setPixelSafe(centerX - 1, centerY + 1, CRGB::Blue);
  setPixelSafe(centerX + 1, centerY + 1, CRGB::Red);

  FastLED.show();

  delay(120);
}

// Set Pixel Safe function
void setPixelSafe(int x, int y, CRGB color) {
  if (x >= 0 && x < 16 && y >= 0 && y < 8) {
    leds[getIndex(x, y)] = color;
  }
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

// Static American Flag function
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

// Waving Flag function
void drawWavingFlag() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      CRGB color;
      
      // Blue canton
      if (x >= CANTON_X_START && x < CANTON_X_END && y < CANTON_HEIGHT) {
        color = CRGB::Blue;
      } else {
        // Flag stripes
        if (y % 2 == 0)
          color = CRGB::Red;
        else
          color = CRGB::White;
      }

      // Generate brightness wave
      uint8_t brightness = 180 + sin8((x * 16) + waveOffset) / 3;

      color.nscale8(brightness);

      leds[getIndex(x, y)] = color;
    }
  }

  FastLED.show();
}

// Twinkling Stars function
void drawTwinklingFlag() {
  drawAmericanFlag(); // draw the normal flag first

  for (int y = 0; y < CANTON_HEIGHT; y++) {
    for (int x = CANTON_X_START; x < CANTON_X_END; x++) {
      if (random8() < 15) {
        uint8_t sparkle = random8(80, 180);
        leds[getIndex(x, y)] = CRGB(sparkle, sparkle, 255);
      }
    }
  }

  FastLED.show();
}

// Patriotic Pulse function
void drawPatrioticPulse() {
  drawAmericanFlag(); // draw the normal flag first

  uint8_t pulseBrightness = 40 + sin8(pulseOffset);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(pulseBrightness);
  }

  FastLED.show();
}

// Indicator update function
void updateStatusIndicators()
{
  if (celebrationMode == CELEBRATION_OFF)
  {
    digitalWrite(flagLED, HIGH);        // flag indicator LED on
    digitalWrite(celebrationLED, LOW);  // celebration indicator LED off
  }
  else
  {
    digitalWrite(flagLED, LOW);         // flag indicator LED off
    digitalWrite(celebrationLED, HIGH); // celebration indicator LED on
  }
}

// Update volume function
void updateVolume() {
  int potValue = analogRead(volumePot);
  int volume = map(potValue, 0, 1023, 0, 30); // re-maps the range of numbers from the potValue to the DFPlayer's range of numbers for volume
  static int lastVolume = -1;                 // the initial value of -1 ensures lastVolume is currently not equal to the volume value

  if (volume != lastVolume) {
    soundFxPlayer.setVol(volume);
    lastVolume = volume;
  }
}