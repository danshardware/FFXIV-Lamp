#ifndef _COLOR_H_
#define _COLOR_H_

#include <Arduino.h>

struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// multipoint gradient
// This will basically have up to 8 entries in the form of position, and a color. 
// Position is uint_8 from 0 to 255, and color is a Color struct.
#define MAX_GRADIENT_POINTS 8
struct Gradient {
  uint8_t num_points;
  struct {
    uint8_t position;
    Color color;
  } points[MAX_GRADIENT_POINTS];
};

// Define a few gradients
// Black Body Radiation
#define GRADIENT_BBR_NUM_POINTS 5
const Gradient gradient_bbr = {
  GRADIENT_BBR_NUM_POINTS,
  {
    {0, {0x00, 0x00, 0x00}},
    {0x08, {0xFF, 0x38, 0x00}},
    {0x3f, {0xFF, 0x93, 0x2C}},
    {0x7f, {0xFF, 0xC1, 0x84}},
    {0xFF, {0xFF, 0xEE, 0xE3}}
  }
};

// Rainbow
#define GRADIENT_RAINBOW_NUM_POINTS 7 
const Gradient gradient_rainbow = {
  GRADIENT_RAINBOW_NUM_POINTS,
  {
    {0, {0x00, 0x00, 0x00}},
    {0x2F, {0xFF, 0x7F, 0x00}},
    {0x5F, {0xFF, 0xFF, 0x00}},
    {0x8F, {0x00, 0xFF, 0x00}},
    {0xBF, {0x00, 0x00, 0xFF}},
    {0xDF, {0x4B, 0x00, 0x82}},
    {0xFF, {0x94, 0x00, 0xFF}}
  }
};

// OK, so we have a gradient, but we need to be able to calculate the color at a given position.
// This is a simple linear interpolation between two points.
Color gradient_color_at(const Gradient *gradient, uint8_t position) {
  // Find the two points that the position is between
  uint8_t i = 1;
  while (i < gradient->num_points && gradient->points[i].position < position) {
    i++;
  }
  if (i == 0) {
    return gradient->points[0].color;
  } else if (i == gradient->num_points) {
    return gradient->points[gradient->num_points - 1].color;
  } else {
    // Linear interpolation
    uint8_t p0 = gradient->points[i - 1].position;
    uint8_t p1 = gradient->points[i].position;
    Color c0 = gradient->points[i - 1].color;
    Color c1 = gradient->points[i].color;
    uint8_t p = position;
    Color c;
    c.r = c0.r + (c1.r - c0.r) * (p - p0) / (p1 - p0);
    c.g = c0.g + (c1.g - c0.g) * (p - p0) / (p1 - p0);
    c.b = c0.b + (c1.b - c0.b) * (p - p0) / (p1 - p0);
    return c;
  }
}

// convert a color to a 32-bit integer
uint32_t color_to_int(const Color *color) {
  return ((uint32_t)color->r << 16) | ((uint32_t)color->g << 8) | color->b;
}

// misc math stolen from FastLED
#define APPLY_FASTLED_RAND16_2053(x) (x << 11) + (x << 2) + x
#define FASTLED_RAND16_13849 ((uint16_t)(13849))

inline uint8_t qadd8(uint8_t i, uint8_t j) {
    unsigned int t = i + j;
    if (t > 255)
        t = 255;
    return t;
}

inline uint8_t qsub8(uint8_t i, uint8_t j) {
    int t = i - j;
    // Serial.print(F("qsub8: "));
    // Serial.print(i);
    // Serial.print(F(" - "));
    // Serial.print(j);
    // Serial.print(F(" = "));
    // Serial.println(t);
    if (t < 0){
        // Serial.println(F("qsub8: returning 0"));
        return 0;
    }
    return t;
}

/// Generate an 8-bit random number
/// @returns random 8-bit number, in the range 0-255
uint8_t random8() {
    static uint16_t rand16seed = APPLY_FASTLED_RAND16_2053(rand16seed) + FASTLED_RAND16_13849;
    // return the sum of the high and low bytes, for better
    //  mixing and non-sequential correlation
    return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                     ((uint8_t)(rand16seed >> 8)));
}

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result, exclusive
uint8_t random8(uint8_t lim) {
    uint8_t r = random8();
    r = (r * lim) >> 8;
    return r;
}

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number, inclusive
/// @param lim the upper bound for the random number, exclusive
uint8_t random8(uint8_t min, uint8_t lim) {
    uint8_t delta = lim - min;
    uint8_t r = random8(delta) + min;
    return r;
}


#endif // _COLOR_H_