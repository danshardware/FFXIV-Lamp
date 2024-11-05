#ifndef _MAIN_CPP_
#define _MAIN_CPP_
#include <Arduino.h>
#include <avr/sleep.h>
#include <Adafruit_NeoPixel.h>
#include "color.h"

#define PIXEL_PIN        11
#define N_LEDS           60
#define PIXELS_PER_LEVEL 8
#define INPUT_1          2
#define INPUT_2          3
#define HEAT_SPREAD      0x80 // how much to attenuate head rising. 0x80 is 50%

// A few operating parameters
#define UPDATE_MS  40          /* how many milliseconds to wait before doing another update. */
#define BRIGHTNESS 16          /* how bright to make this overall 0-255*/
#define MINIMUM_BASE_HEAT 0x10 /* The leasat amount of light at the base */

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

// The heat map
uint8_t heat[N_LEDS];

// Fades away all the LEDs by 1/4
void fade_all_leds()
{
  for (int i = 0; i < N_LEDS; i++)
  {
    // heat[i] = (heat[i] >> 1) + (heat[i] >> 2);
    // heat[i] = heat[i] - (heat[i] >> 2);
    heat[i] = qsub8(heat[i], 2);
  }
}

// Add random heat to a random LED
void add_random_heat()
{
  int led = random(PIXELS_PER_LEVEL * 2);
  heat[led] = qadd8(heat[led], random8(0x40, 0xE0));
}

// Apply a blur effect to the LEDs, and respect the rows
void smear()
{
  uint8_t new_heat[N_LEDS];
  // int num_blurred = PIXELS_PER_LEVEL * 2;
  for (int i = 0; i < N_LEDS; i++)
  {
    if (i % PIXELS_PER_LEVEL == 0)
    {
      // Beginning of a row, so blur with the next and pixel at end of the row
      uint16_t sum = heat[i] + heat[i] + heat[(i + 1) % N_LEDS] +  heat[(i + PIXELS_PER_LEVEL - 1) % N_LEDS];
      sum = sum >> 2;
      new_heat[i] = sum;
    }
    else if (i % PIXELS_PER_LEVEL == PIXELS_PER_LEVEL - 1)
    {
      // end of a row, so blur with the previous and pixel at the beginning of the row
      uint16_t sum = heat[i] + heat[i] + heat[(i + N_LEDS - 1) % N_LEDS] +  heat[(i - PIXELS_PER_LEVEL + 1) % N_LEDS];
      sum = sum >> 2;
      new_heat[i] = sum;
    }
    else
    {
      uint16_t sum = heat[i] + heat[i] + heat[(i + 1) % N_LEDS] + heat[(i - 1) % N_LEDS];
      sum = sum >> 2;
      new_heat[i] = sum;
      // Print the values that went into this calculation
      // Serial.print(F("Pixel Values: "));
      // Serial.print(heat[i]);
      // Serial.print(F(", "));
      // Serial.print(heat[(i + 1) % N_LEDS]);
      // Serial.print(F(", "));
      // Serial.println(heat[(i - 1) % N_LEDS]);
      // Serial.print(F("sum: "));
      // Serial.print(sum);
      // Serial.print(F(", new_heat["));
      // Serial.print(i);
      // Serial.print(F("]: "));
      // Serial.println(new_heat[i]); 
    }
    
  }

  for (int i = 0; i < N_LEDS; i++)
  {
    heat[i] = new_heat[i];
  }
}

// Ensure the bottom 2 rows doesn't get too cold
void ensure_bottom_heat()
{
  for (int i = 0; i < PIXELS_PER_LEVEL * 2; i++)
  {
    if (heat[i] < MINIMUM_BASE_HEAT)
    {
      heat[i] = MINIMUM_BASE_HEAT;
    }
  }
}

// Make the heat rise
void heat_rises(){
  for (int i = PIXELS_PER_LEVEL; i < N_LEDS; i++)
  {
    // Add heat from the pixel under this one and half the pixel to the down-left and down-right.
    // uint16_t sum = heat[i] + heat[(i - PIXELS_PER_LEVEL) % N_LEDS] + heat[(i - PIXELS_PER_LEVEL + 1) % N_LEDS] + heat[(i - PIXELS_PER_LEVEL - 1) % N_LEDS];
    uint16_t sum = heat[i] + heat[i] + heat[(i - PIXELS_PER_LEVEL) % N_LEDS] + (heat[(i - PIXELS_PER_LEVEL + 1) % N_LEDS] >> 1) + (heat[(i - PIXELS_PER_LEVEL - 1) % N_LEDS] >> 1);
    // divide by 4
    sum = sum >> 2;
    sum *= HEAT_SPREAD;
    sum = sum >> 8;
    heat[i] = sum;
    // heat[i] = qsub8(sum, 8);
  }
}
// Update all the LEDs
void update_leds()
{
  for (int i = 0; i < N_LEDS; i++)
  {
    Color c = gradient_color_at(&gradient_bbr, heat[i]);
    uint32_t color_int = color_to_int(&c);
    strip.setPixelColor(i, color_int);
  }
  strip.show();
}

void zzz(){
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  sleep_mode();
  sleep_disable();
}

void setup()
{
  memset(heat, 0, sizeof(heat));
  memset(heat, 0x20, PIXELS_PER_LEVEL * 2);
  pinMode(INPUT_1, INPUT_PULLUP);
  pinMode(INPUT_2, INPUT_PULLUP);
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
}

void loop()
{
  auto last_update_millis = millis();
  int counter = 0;
  while (1)
  {
    if (millis() - last_update_millis > UPDATE_MS)
    {
      last_update_millis = millis();
      // Flicker
      if (digitalRead(INPUT_1) == LOW)
      {
        add_random_heat();
      
        fade_all_leds();
        if (counter > 5){
          counter = 0;
          add_random_heat();
        }
        counter++;
        heat_rises();
        smear();
        ensure_bottom_heat();
        update_leds();
      } else if (digitalRead(INPUT_2) == LOW) {
        // Steady
        memset(heat, 0x80, sizeof(heat));
        update_leds();
        while (digitalRead(INPUT_2) == LOW)
        {
          zzz();
        }
      } else {
        // Off, middle position, nothing shorted to ground
        memset(heat, 0x00, sizeof(heat));
        update_leds();
        while (digitalRead(INPUT_2) == HIGH && digitalRead(INPUT_1) == HIGH)
        {
          zzz();
        }
      }
    }
    // Put the Arduino to sleep to save power
    zzz();
  }
}
#endif // _MAIN_CPP_