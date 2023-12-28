/**
 * @file ESP32_Stair_Lights.ino
 *
 * @details
 * This Arduino project controls a WS2812B addressable LED strip installed on the wall
 * next to a staircase. The lighting is adjusted based on ambient light sensed by a
 * Light-Dependent Resistor (LDR), providing a dynamic brightness to the LEDs so they are
  dimmer when it is darker and brighter when there is more light.
 *
 * Motion sensors located at the top and bottom of the stairs detect movement. When motion
 * is detected, the LEDs transition from a base hue incrementing rainbow pattern to a custom sequence:
 * 1. All LEDs light up at half brightness in purple.
 * 2. LEDs sequentially populate with an increased brightness in medium purple.
 *
 * @note
 * Components:
 * - One D1 Mini NodeMCU ESP32
 * - WS2812B Addressable LED strip
 * - Light-Dependent Resistor (LDR) for ambient light sensing
 * - Two PIR motion sensors (AM312) for detecting movement at the top and bottom of the stairs
 *
 * Functionalities:
 * - Dynamic adjustment of LED brightness based on ambient light conditions.
 * - Motion-triggered transitions to specific lighting sequences.
 *
 * @attention
 * Author: [Bob Hampton]
 * Date: [12/27/2023]
 *
 * @section credits Credits
 * Special thanks to Giovanni Aggiustatutto for their writeup on constructing the hexagons:
 * https://www.instructables.com/DIY-Wooden-Nanoleaf-Light-Panels/
 */

// Include libraries for gesture sensor and WS2812B LEDs
#define FASTLED_INTERNAL            // Surpress the annoying banner that FastLED prints out every time you compile
#include <FastLED.h>

// Set the number of LEDs.
const int NUM_LEDS = 192;           // Ignore this comment. 99 for my own test strip that I have setup on my desk

// Set the pin numbers for the data LED, LDR, and bottom/top PIRs
const int DATA_PIN_LEDS = 16;
const int PIR_BOTTOM_PIN = 17;
const int PIR_TOP_PIN = 27;
const int LDR_PIN = 34;

// Set upper/lower bounds for input/output
const int LDR_MIN = 0;              // Lower bound of the input range (0-4095)
const int LDR_MAX = 4095;           // Upper bound of the input range (0-4095)
const int MIN_BRIGHT = 5;           // Lower bound of the output range (0-255)
const int MAX_BRIGHT = 100;         // Upper bound of the output range (0-255)

uint8_t paletteIndex = 0;

int halfBright;

// Create LED object and palette
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending;

void setup() {
  delay(3000); // power-up safety delay
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, DATA_PIN_LEDS, GRB>(leds, NUM_LEDS);

  currentPalette = RainbowColors_p; // Use palette available from FastLED lib
  currentBlending = LINEARBLEND;
}

void loop() {
  // Read LDR value (ranges from 0 - 4095)
  int LDR = analogRead(LDR_PIN);
  // Map the analog value to brightness range
  int brightness = map(LDR, LDR_MIN, LDR_MAX, MIN_BRIGHT, MAX_BRIGHT); 

  // Uncomment next line to make sure your LDR is working correctly
  // Serial.println(brightness);
  
  // Clear any data in the LED strip
  FastLED.clear();
  // Turn on the LED strip with ambient brightness taken into account
  fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, currentPalette, brightness, LINEARBLEND);
  EVERY_N_MILLISECONDS (10) {   // Raise or lower this number to set the speed
    paletteIndex++;
  }
  FastLED.show();

  // Get the motion sensors states (HIGH or LOW)
  int pirBottom = digitalRead(PIR_BOTTOM_PIN);
  int pirTop = digitalRead(PIR_TOP_PIN);

  // Check for motion at the top of the stairs
  if (pirTop == HIGH) {
    // Reduce the brightness and have all LEDs turn purple
    halfBright = brightness /= 4; // Not technically half, but /2 didn't seem like half
    FastLED.setBrightness(halfBright);
    fill_solid(leds, NUM_LEDS, CRGB::Purple);

    // Iterate over the LEDs with an increased brightness from the top to the bottom of the stairs
    for (int i = 0; i < NUM_LEDS; i++) {
      FastLED.setBrightness(brightness);
      leds[i] = CRGB::MediumPurple;
      FastLED.show();
      delay(15);
    }
    delay(5000);
    if (pirBottom == HIGH || pirTop == HIGH) {
      delay(5000);
    }
    FastLED.clear();
  }

  // Check for motion at the bottom of the stairs
  else if (pirBottom == HIGH) {
    // Reduce the brightness and have all LEDs turn purple
    halfBright = brightness /= 4;
    FastLED.setBrightness(halfBright);
    fill_solid(leds, NUM_LEDS, CRGB::Purple);

    // Iterate over the LEDs with an increased brightness from the bottom to the top of the stairs
    for (int i = (NUM_LEDS) - 1; i >= 0; i--) {
      FastLED.setBrightness(brightness);
      leds[i] = CRGB::MediumPurple;
      FastLED.show();
      delay(15);
    }
    delay(5000);
    if (pirBottom == HIGH || pirTop == HIGH) {
      delay(5000);
    }
  }

  // Clear any data left in the LEDs
  FastLED.clear();    

}
