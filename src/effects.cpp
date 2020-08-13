#include "globals.h"
#include "FastLED.h"
#include "tileprotocol.h"
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
TBlendType    blendType;
TBlendType currentBlending; // Current blending type

void juggle()
{
    static uint8_t    numdots = 4; // Number of dots in use.
    static uint8_t   faderate = 2; // How long should the trails be. Very low value = longer trails.
    static uint8_t     hueinc = 255 / numdots - 1; // Incremental change in hue between each dot.
    static uint8_t    thishue = 0; // Starting hue.
    static uint8_t     curhue = 0; // The current hue
    static uint8_t    thissat = 255; // Saturation of the colour.
    static uint8_t thisbright = 255; // How bright should the LED/display be.
    static uint8_t   basebeat = 5; // Higher = faster movement.

    static uint8_t lastSecond = 99;  // Static variable, means it's only defined once. This is our 'debounce' variable.
    uint8_t secondHand = (millis() / 1000) % 30; // IMPORTANT!!! Change '30' to a different value to change duration of the loop.

    if (lastSecond != secondHand) { // Debounce to make sure we're not repeating an assignment.
        lastSecond = secondHand;
        switch (secondHand) {
        case  0: numdots = 1; basebeat = 20; hueinc = 16; faderate = 2; thishue = 0; break; // You can change values here, one at a time , or altogether.
        case 10: numdots = 4; basebeat = 10; hueinc = 16; faderate = 8; thishue = 128; break;
        case 20: numdots = 8; basebeat = 3; hueinc = 0; faderate = 8; thishue = random8(); break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
        case 30: break;
        }
    }

    // Several colored dots, weaving in and out of sync with each other
    curhue = thishue; // Reset the hue values.
    fadeToBlackBy(leds, num_tiles, faderate);
    for (int i = 0; i < numdots; i++) {
        //beat16 is a FastLED 3.1 function
        leds[beatsin16(basebeat + i + numdots, 0, num_tiles)] += CHSV(gHue + curhue, thissat, thisbright);
        curhue += hueinc;
    }
}

void pride() 
{
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;
 
  uint8_t sat8 = beatsin88( 87, 220, 250);
  uint8_t brightdepth = beatsin88( 341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88( 203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 1, 3000);
  
  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;
  
  for( uint16_t i = 0; i < num_tiles; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16( brightnesstheta16  ) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);
    
    CRGB newcolor = CHSV( hue8, sat8, bri8);
    
    nblend( leds[i], newcolor, 64);
  }
}

void randomPaletteFades()
{
    uint16_t i = random16(0, (NUM_LEDS - 1)); // Pick a random LED
    {
        uint8_t colorIndex = random8(0, 255); // Pick a random color (from palette)
        if (CRGB(0, 0, 0) == CRGB(leds[i])) // Only set new color to LED that is off
        {
            leds[i] = ColorFromPalette(palettes[currentPaletteIndex], colorIndex, 255, currentBlending);
            blur1d(leds, NUM_LEDS, 32); // Blur colors with neighboring LEDs
        }
    }
    fadeToBlackBy(leds, NUM_LEDS, 8); // Slowly fade LEDs to black
}

void colorwaves(CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette)
{
    static uint16_t sPseudotime = 0;
    static uint16_t sLastMillis = 0;
    static uint16_t sHue16 = 0;

    // uint8_t sat8 = beatsin88( 87, 220, 250);
    uint8_t brightdepth = beatsin88(341, 96, 224);
    uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
    uint8_t msmultiplier = beatsin88(147, 23, 60);

    uint16_t hue16 = sHue16;//gHue * 256;
    uint16_t hueinc16 = beatsin88(113, 300, 1500);

    uint16_t ms = millis();
    uint16_t deltams = ms - sLastMillis;
    sLastMillis = ms;
    sPseudotime += deltams * msmultiplier;
    sHue16 += deltams * beatsin88(400, 5, 9);
    uint16_t brightnesstheta16 = sPseudotime;

    for (uint16_t i = 0; i < numleds; i++) {
        hue16 += hueinc16;
        uint8_t hue8 = hue16 / 256;
        uint16_t h16_128 = hue16 >> 7;
        if (h16_128 & 0x100) {
            hue8 = 255 - (h16_128 >> 1);
        }
        else {
            hue8 = h16_128 >> 1;
        }

        brightnesstheta16 += brightnessthetainc16;
        uint16_t b16 = sin16(brightnesstheta16) + 32768;

        uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
        uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
        bri8 += (255 - brightdepth);

        uint8_t index = hue8;
        index = scale8(index, 240);

        CRGB newcolor = ColorFromPalette(palette, index, bri8);

        uint16_t pixelnumber = i;
        pixelnumber = (num_tiles * 3 - 1) - pixelnumber;
        for (int i2 = 0; i2 < 1; i2++)
        {
            nblend(leds[pixelnumber + i2], newcolor, 128);
        }
    }
}

void rainbow()
{
    for (int i = 0; i < num_tiles; i++)
    {
        uint8_t myHue = (gHue + i * (255 / num_tiles));
        gHue = gHue > 255 ? gHue - 255 : gHue;
        fill_solid(leds + i * 1, 1, CHSV(myHue, 255, 255));
    }
}

void bpm()
{
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t beat = beatsin8(speed, 64, 255);
    CRGBPalette16 palette = palettes[currentPaletteIndex];
    for (int i = 0; i < num_tiles; i++) {
        for (int i2 = 0; i2 < 1; i2++)leds[i * 1 + i2] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
    }
}

void confetti()
{
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, 34, 1);
    int pos = random16(34);
    int val = gHue + random8(64);
    for (int i = 0; i <= 1; i++)
    {

        leds[i + pos] += ColorFromPalette(palettes[currentPaletteIndex], val);
    }
}

unsigned long lastFrameTime = 0;
void display() {
    for(int i = 0; i < num_tiles; i++) {
            setColor(i+1, leds[i].r, leds[i].g, leds[i].b);
        }
}

void solidColor(CRGB color) {
  for(int i = 0; i < num_tiles; i++) {
    leds[i] = color;
  }
}

void off() {
    solidColor(CRGB(0x000000));
}