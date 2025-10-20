#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <math.h>
#include "EmojiProgmem.h"

#define LED_PIN 4
#define LED_WIDTH 8
#define LED_HEIGHT 8
#define NUM_LEDS (LED_WIDTH * LED_HEIGHT)
#define BAT_VOL_ENABLE 7
#define BAT_VOL_ADC 5

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_RGB + NEO_KHZ800);
Preferences prefs;

uint8_t currentAnimation = 0;
const uint8_t totalAnimations = 10;

int XY(int x, int y)
{
  int row_start = y * LED_WIDTH;
  return row_start + (7 - x);
}

void clearStrip()
{
  for (int i = 0; i < NUM_LEDS; i++)
    strip.setPixelColor(i, 0);
  strip.show();
}

void rainbowCycle()
{
  uint16_t hue = 0;
  bool invertNext = false;

  for (;;)
  {
    for (uint8_t y = 0; y < LED_HEIGHT; y++)
    {
      for (uint8_t x = 0; x < LED_WIDTH; x++)
      {
        uint16_t i = XY(x, y);

        uint32_t color = strip.ColorHSV(hue, 255, 255);
        strip.setPixelColor(i, color);
        strip.show();

        hue += 400;
        if (hue > 65535)
          hue -= 65535;

        delay(20);
      }
    }

    for (int y = LED_HEIGHT - 1; y >= 0; y--)
    {
      for (int x = LED_WIDTH - 1; x >= 0; x--)
      {
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

void matrixRain()
{
  uint8_t drops[LED_WIDTH] = {0};
  for (int frame = 0; frame < 400; frame++)
  {
    clearStrip();
    for (uint8_t x = 0; x < LED_WIDTH; x++)
    {
      if (random(10) < 2)
        drops[x] = 0;
      if (drops[x] < LED_HEIGHT)
      {
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

void purpleWaterfall()
{
  uint8_t drops[LED_WIDTH];
  for (uint8_t x = 0; x < LED_WIDTH; x++)
  {
    drops[x] = random(LED_HEIGHT);
  }

  for (int frame = 0; frame < 600; frame++)
  {

    // fade all pixels
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
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
    for (uint8_t x = 0; x < LED_WIDTH; x++)
    {
      if (random(10) < 7)
      { // higher = denser rain
        uint8_t y = drops[x];
        uint8_t r = 150 + random(0, 100);
        uint8_t g = 0;
        uint8_t b = 150 + random(0, 100);

        // invert Y
        strip.setPixelColor(XY(x, LED_HEIGHT - 1 - y), strip.Color(r, g, b));

        drops[x]++;
        if (drops[x] >= LED_HEIGHT)
          drops[x] = 0;
      }
    }

    strip.show();
    delay(50);
  }
}

void fireflies()
{
  struct Firefly
  {
    float x, y, dx, dy;
  };
  const int count = 6;
  Firefly flies[count];
  for (int i = 0; i < count; i++)
  {
    flies[i] = {random(LED_WIDTH), random(LED_HEIGHT),
                ((float)random(-100, 100)) / 200.0,
                ((float)random(-100, 100)) / 200.0};
  }

  for (int frame = 0; frame < 400; frame++)
  {
    clearStrip();
    for (int i = 0; i < count; i++)
    {
      flies[i].x += flies[i].dx;
      flies[i].y += flies[i].dy;
      if (flies[i].x < 0 || flies[i].x >= LED_WIDTH)
        flies[i].dx *= -1;
      if (flies[i].y < 0 || flies[i].y >= LED_HEIGHT)
        flies[i].dy *= -1;
      uint16_t hue = (i * 10000 + frame * 300) % 65536;
      strip.setPixelColor(XY((int)flies[i].x, (int)flies[i].y),
                          strip.ColorHSV(hue, 255, 255));
    }
    strip.show();
    delay(50);
  }
}

void sparkExplosion()
{
  for (int burst = 0; burst < 10; burst++)
  {
    clearStrip();
    uint8_t cx = random(LED_WIDTH);
    uint8_t cy = random(LED_HEIGHT);
    uint32_t color = strip.ColorHSV(random(0, 65535), 255, 255);
    strip.setPixelColor(XY(cx, cy), color);
    strip.show();
    delay(50);

    for (int r = 1; r < 5; r++)
    {
      for (int dy = -r; dy <= r; dy++)
      {
        for (int dx = -r; dx <= r; dx++)
        {
          int nx = cx + dx;
          int ny = cy + dy;
          if (nx >= 0 && nx < LED_WIDTH && ny >= 0 && ny < LED_HEIGHT)
          {
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

float ReadVoltage()
{
  digitalWrite(BAT_VOL_ENABLE, LOW);
  delay(10);

  int raw = analogRead(BAT_VOL_ADC);

  // Convert raw ADC reading => volts on ADC pin
  float v_adc = 0.05 + (raw / 4095.0) * 1.1;
  float v_bat = v_adc / 0.2326;

  // Serial.printf("RAW: %d  |  Vadc: %.3f V  |  Vbat: %.3f V\n", raw, v_adc, v_bat);

  digitalWrite(BAT_VOL_ENABLE, HIGH);

  return v_bat;
}

void rainbowWorm()
{
  const int WORM_LENGTH = 5;

  // Worm position history
  int wormX[WORM_LENGTH];
  int wormY[WORM_LENGTH];

  // Start worm in center
  wormX[0] = LED_WIDTH / 2;
  wormY[0] = LED_HEIGHT / 2;
  for (int i = 1; i < WORM_LENGTH; i++)
  {
    wormX[i] = wormX[0];
    wormY[i] = wormY[0];
  }

  // Random movement direction
  int dx = 1;
  int dy = 0;

  uint16_t hue = 0;

  for (int frame = 0; frame < 1000; frame++)
  {
    clearStrip();

    // Move worm head
    wormX[0] += dx;
    wormY[0] += dy;

    // Bounce off walls
    if (wormX[0] < 0)
    {
      wormX[0] = 0;
      dx = random(0, 2);
      dy = random(-1, 2);
    }
    if (wormX[0] >= LED_WIDTH)
    {
      wormX[0] = LED_WIDTH - 1;
      dx = random(-1, 1);
      dy = random(-1, 2);
    }
    if (wormY[0] < 0)
    {
      wormY[0] = 0;
      dx = random(-1, 2);
      dy = random(0, 2);
    }
    if (wormY[0] >= LED_HEIGHT)
    {
      wormY[0] = LED_HEIGHT - 1;
      dx = random(-1, 2);
      dy = random(-1, 1);
    }

    // Shift tail positions
    for (int i = WORM_LENGTH - 1; i > 0; i--)
    {
      wormX[i] = wormX[i - 1];
      wormY[i] = wormY[i - 1];
    }

    // Draw worm
    for (int i = 0; i < WORM_LENGTH; i++)
    {
      float fade = 1.0 - (float)i / WORM_LENGTH; // tail fades
      uint16_t wormHue = (hue + i * 3000) % 65536;
      uint32_t color = strip.ColorHSV(wormHue, 255, (uint8_t)(255 * fade));
      strip.setPixelColor(XY(wormX[i], wormY[i]), color);
    }

    strip.show();

    hue += 1000; // move rainbow forward
    if (hue > 65535)
      hue -= 65535;

    delay(100);
  }
}

void rainbowLineCycle()
{
  uint16_t hue = 0;

  for (;;)
  { // continuous loop
    // ---- FORWARD: rainbow fill ----
    for (uint8_t y = 0; y < LED_HEIGHT; y++)
    {
      for (uint8_t x = 0; x < LED_WIDTH; x++)
      {
        uint16_t i = XY(x, y);
        uint32_t color = strip.ColorHSV(hue, 255, 255);
        strip.setPixelColor(i, color);
        strip.show();

        hue += 400; // color speed
        if (hue > 65535)
          hue -= 65535;
        delay(20);
      }
    }

    // ---- BACKWARD: invert colors line by line ----
    for (int y = LED_HEIGHT - 1; y >= 0; y--)
    {
      for (int x = LED_WIDTH - 1; x >= 0; x--)
      {
        uint16_t i = XY(x, y);
        uint32_t c = strip.getPixelColor(i);
        uint8_t r = (uint8_t)(c >> 16);
        uint8_t g = (uint8_t)(c >> 8);
        uint8_t b = (uint8_t)(c);

        // invert RGB
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;

        strip.setPixelColor(i, strip.Color(r, g, b));
        strip.show();
        delay(20);
      }
    }

    // Reset hue for new forward rainbow
    hue = 0;
  }
}

void rainbowSpiral()
{
  const int TRAIL_LENGTH = 6;
  const int totalPixels = NUM_LEDS;

  // --- Create spiral path from center outwards ---
  uint16_t path[NUM_LEDS];
  int idx = 0;
  int x = LED_WIDTH / 2 - 1;
  int y = LED_HEIGHT / 2 - 1;
  int dx = 1, dy = 0;
  int segmentLength = 1;
  int segmentPassed = 0;
  int steps = 0;
  int turns = 0;

  // Generate spiral coordinates
  while (idx < NUM_LEDS)
  {
    path[idx++] = XY(x, y);
    x += dx;
    y += dy;
    segmentPassed++;
    if (segmentPassed == segmentLength)
    {
      segmentPassed = 0;
      int temp = dx;
      dx = -dy;
      dy = temp;
      turns++;
      if (turns % 2 == 0)
        segmentLength++;
    }
    // Stop if outside matrix
    if (x < 0 || y < 0 || x >= LED_WIDTH || y >= LED_HEIGHT)
      break;
  }

  uint16_t hue = 0;

  // --- Animate spiral movement ---
  for (int frame = 0; frame < 1000; frame++)
  {
    clearStrip();
    hue += 500; // slowly shift color hue

    // head moves forward
    int head = frame % idx;

    for (int t = 0; t < TRAIL_LENGTH; t++)
    {
      int pos = head - t;
      if (pos < 0)
        pos += idx;

      // fade brightness for trail
      float fade = 1.0 - (float)t / TRAIL_LENGTH;
      uint16_t trailHue = (hue + t * 2000) % 65536;
      uint32_t color = strip.ColorHSV(trailHue, 255, (uint8_t)(255 * fade));
      strip.setPixelColor(path[pos], color);
    }

    strip.show();
    delay(40);
  }
}

void spiralSwirl()
{
  for (int frame = 0; frame < 500; frame++)
  {
    for (uint8_t y = 0; y < LED_HEIGHT; y++)
    {
      for (uint8_t x = 0; x < LED_WIDTH; x++)
      {
        float cx = x - (LED_WIDTH / 2.0);
        float cy = y - (LED_HEIGHT / 2.0);
        float angle = atan2(cy, cx) + frame * 0.05;
        float dist = sqrt(cx * cx + cy * cy);
        uint16_t hue = (uint16_t)((angle * 4000) + dist * 8000);
        strip.setPixelColor(XY(x, y), strip.ColorHSV(hue, 255, 255));
      }
    }
    strip.show();
    delay(30);
  }
}

void colorWaves()
{
  for (int frame = 0; frame < 400; frame++)
  {
    for (uint8_t y = 0; y < LED_HEIGHT; y++)
    {
      for (uint8_t x = 0; x < LED_WIDTH; x++)
      {
        float wave = sin((x + frame * 0.3) * 0.5) + cos((y + frame * 0.2) * 0.3);
        uint16_t hue = (uint16_t)((wave + 2) * 16000);
        strip.setPixelColor(XY(x, y), strip.ColorHSV(hue, 255, 255));
      }
    }
    strip.show();
    delay(30);
  }
}

void plasma()
{
  for (int frame = 0; frame < 500; frame++)
  {
    for (uint8_t y = 0; y < LED_HEIGHT; y++)
    {
      for (uint8_t x = 0; x < LED_WIDTH; x++)
      {
        float v = sin(x * 0.3 + frame * 0.1) + sin(y * 0.3 + frame * 0.1);
        uint16_t hue = (uint16_t)((v + 2) * 16000);
        strip.setPixelColor(XY(x, y), strip.ColorHSV(hue, 255, 255));
      }
    }
    strip.show();
    delay(25);
  }
}

void RenderBatVol1()
{

  float voltage = ReadVoltage();

  int whole = int(voltage);
  int decimal = int((voltage - whole) * 10);

  strip.clear();

  uint32_t color = strip.Color(
      (4.2 - voltage) * 255,
      (voltage - 3.3) * 255,
      0);

  int y = LED_HEIGHT - 4;
  // whole voltage
  for (int v = 0; v < whole; v++)
  {
    int x = v * 2;
    strip.setPixelColor(XY(x, y), color);
    strip.setPixelColor(XY(x + 1, y), color);
    strip.setPixelColor(XY(x, y + 1), color);
    strip.setPixelColor(XY(x + 1, y + 1), color);
  }

  // decimal places
  if (decimal > 0)
  {
    int x = 0;
    int y = LED_HEIGHT - 1;
    for (int i = 0; i < decimal; i++)
    {
      x = i;
      if (i >= LED_WIDTH)
      {
        y += 1;
        x = i - LED_WIDTH;
      }
      strip.setPixelColor(XY(x, y), strip.Color(0, 0, 255)); // blue dot for .x
    }
  }

  strip.show();
  delay(500);
}

void runAnimation(uint8_t index)
{
  switch (index)
  {
  case 0:
    rainbowCycle();
    break;
  case 1:
    purpleWaterfall();
    break;
  case 2:
    matrixRain();
    break;
  case 3:
    sparkExplosion();
    break;
  case 4:
    fireflies();
    break;
  case 5:
    renderEmoji(strip, &SMILEY);
    break;
  case 6:
    renderEmoji(strip, &HEART);
    break;
  case 7:
    RenderBatVol1();
    break;
  case 8:
    plasma();
    break;
  case 9:
    rainbowSpiral();
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(5);
  strip.show();
  prefs.begin("ledapp", false);

  currentAnimation = prefs.getUChar("animIndex", 0);
  currentAnimation = (currentAnimation + 1) % totalAnimations;
  prefs.putUChar("animIndex", currentAnimation);
  prefs.end();

  pinMode(BAT_VOL_ENABLE, OUTPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(BAT_VOL_ADC, ADC_2_5db);

  randomSeed(esp_random());
}

void loop()
{

  runAnimation(currentAnimation);
}
