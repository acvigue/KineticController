#include "globals.h"
#include "FastLED.h"
#include "tileprotocol.h"

uint8_t gHue = 0; // rotating "base color" used by many of the patterns


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

void display() {
    for(int i = 0; i < num_tiles; i++) {
        setColor(i+1, leds[i].r, leds[i].g, leds[i].b);
    }
}
/*
void heatMap(CRGBPalette16 palette, bool up)
{
    fill_solid(leds, NUM_LEDS, CRGB::Black);

    // Add entropy to random number generator; we use a lot of it.
    random16_add_entropy(random(256));

    // Array of temperature readings at each simulation cell
    static byte heat[34];

    byte colorindex;

    // Step 1.  Cool down every cell a little
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint16_t k = NUM_LEDS - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking) {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint16_t j = 0; j < NUM_LEDS; j++) {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        colorindex = scale8(heat[j], 190);

        CRGB color = ColorFromPalette(palette, colorindex);

        if (up) {
            leds[j] = color;
        }
        else {
            leds[(NUM_LEDS - 1) - j] = color;
        }
    }
}
*/

void solidColor(CRGB color) {
  for(int i = 0; i < num_tiles; i++) {
    leds[i] = color;
  }
}

void off() {
    solidColor(CRGB(0x000000));
}