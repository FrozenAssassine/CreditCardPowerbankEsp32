#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <math.h>
#include "EmojiProgmem.h"

#define LED_PIN     4
#define LED_WIDTH   8
#define LED_HEIGHT  8
#define NUM_LEDS    (LED_WIDTH * LED_HEIGHT)

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_RGB + NEO_KHZ800);
Preferences prefs;

uint8_t currentAnimation = 0;
const uint8_t totalAnimations = 7;

uint16_t XY(uint8_t x, uint8_t y) {
  if (y % 2 == 0) return y * LED_WIDTH + x;
  else return y * LED_WIDTH + (LED_WIDTH - 1 - x);
}

void clearStrip() {
  for (int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, 0);
  strip.show();
}

void rainbowCycle() {
  uint16_t hue = 0;
  bool invertNext = false; 

  for (;;) {
    for (uint8_t y = 0; y < LED_HEIGHT; y++) {
      for (uint8_t x = 0; x < LED_WIDTH; x++) {
        uint16_t i = XY(x, y);

        uint32_t color = strip.ColorHSV(hue, 255, 255);
        strip.setPixelColor(i, color);
        strip.show();

        hue += 400;
        if (hue > 65535) hue -= 65535;

        delay(20);
      }
    }

    for (int y = LED_HEIGHT - 1; y >= 0; y--) {
      for (int x = LED_WIDTH - 1; x >= 0; x--) {
        uint16_t i = XY(x, y);
        uint32_t c = strip.getPixelColor(i);
        uint8_t r = (uint8_t)(c >> 16);
        uint8_t g = (uint8_t)(c >> 8);
        uint8_t b = (uint8_t)(c);

        r = 255 - r;
        g = 255 - g;
        b = 255 - b;

        strip.setPixelColor(i, strip.Color(r, g, b));
        strip.show();
        delay(20);
      }
    }

    hue = 0;
  }
}

void matrixRain() {
  uint8_t drops[LED_WIDTH] = {0};
  for (int frame = 0; frame < 400; frame++) {
    clearStrip();
    for (uint8_t x = 0; x < LED_WIDTH; x++) {
      if (random(10) < 2) drops[x] = 0;
      if (drops[x] < LED_HEIGHT) {
        // invert Y: LED_HEIGHT - 1 - drops[x]
        strip.setPixelColor(XY(x, LED_HEIGHT - 1 - drops[x]), strip.Color(0, 255, 0));
        if (drops[x] > 0) 
          strip.setPixelColor(XY(x, LED_HEIGHT - drops[x]), strip.Color(0, 100, 0));
        drops[x]++;
      }
    }
    strip.show();
    delay(80);
  }
}

void purpleWaterfall() {
  uint8_t drops[LED_WIDTH];
  for (uint8_t x = 0; x < LED_WIDTH; x++) {
    drops[x] = random(LED_HEIGHT);
  }

  for (int frame = 0; frame < 600; frame++) {

    // fade all pixels
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      uint32_t c = strip.getPixelColor(i);
      uint8_t r = (uint8_t)(c >> 16);
      uint8_t g = (uint8_t)(c >> 8);
      uint8_t b = (uint8_t)(c);
      r = r * 0.6;
      g = g * 0.6;
      b = b * 0.6;
      strip.setPixelColor(i, r, g, b);
    }

    // draw drops downward
    for (uint8_t x = 0; x < LED_WIDTH; x++) {
      if (random(10) < 7) {  // higher = denser rain
        uint8_t y = drops[x];
        uint8_t r = 150 + random(0, 100);
        uint8_t g = 0;
        uint8_t b = 150 + random(0, 100);

        // invert Y
        strip.setPixelColor(XY(x, LED_HEIGHT - 1 - y), strip.Color(r, g, b));

        drops[x]++;
        if (drops[x] >= LED_HEIGHT) drops[x] = 0;
      }
    }

    strip.show();
    delay(50);
  }
}

void fireflies() {
  struct Firefly { float x, y, dx, dy; };
  const int count = 6;
  Firefly flies[count];
  for (int i = 0; i < count; i++) {
    flies[i] = {random(LED_WIDTH), random(LED_HEIGHT),
                ((float)random(-100,100))/200.0,
                ((float)random(-100,100))/200.0};
  }

  for (int frame = 0; frame < 400; frame++) {
    clearStrip();
    for (int i = 0; i < count; i++) {
      flies[i].x += flies[i].dx;
      flies[i].y += flies[i].dy;
      if (flies[i].x < 0 || flies[i].x >= LED_WIDTH) flies[i].dx *= -1;
      if (flies[i].y < 0 || flies[i].y >= LED_HEIGHT) flies[i].dy *= -1;
      uint16_t hue = (i * 10000 + frame * 300) % 65536;
      strip.setPixelColor(XY((int)flies[i].x, (int)flies[i].y),
                          strip.ColorHSV(hue, 255, 255));
    }
    strip.show();
    delay(50);
  }
}

void sparkExplosion() {
  for (int burst = 0; burst < 10; burst++) {
    clearStrip();
    uint8_t cx = random(LED_WIDTH);
    uint8_t cy = random(LED_HEIGHT);
    uint32_t color = strip.ColorHSV(random(0,65535), 255, 255);
    strip.setPixelColor(XY(cx, cy), color);
    strip.show();
    delay(50);

    for (int r = 1; r < 5; r++) {
      for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
          int nx = cx + dx;
          int ny = cy + dy;
          if (nx >= 0 && nx < LED_WIDTH && ny >= 0 && ny < LED_HEIGHT) {
            strip.setPixelColor(XY(nx, ny), color);
          }
        }
      }
      strip.show();
      delay(80);
      clearStrip();
    }
  }
}


void runAnimation(uint8_t index) {
  switch (index) {
    case 0: rainbowCycle(); break;
    case 1: purpleWaterfall(); break;
    case 2: matrixRain(); break;
    case 3: sparkExplosion(); break;
    case 4: fireflies(); break;
    case 5: renderEmoji(strip, &SMILEY); break;
    case 6: renderEmoji(strip, &HEART); break;
  }
}


void setup() {
  strip.begin();
  strip.setBrightness(5);
  strip.show();
  prefs.begin("ledapp", false);

  currentAnimation = prefs.getUChar("animIndex", 0);
  currentAnimation = (currentAnimation + 1) % totalAnimations;
  prefs.putUChar("animIndex", currentAnimation);
  prefs.end();

  randomSeed(esp_random());
}

void loop() {
  runAnimation(currentAnimation);
}
