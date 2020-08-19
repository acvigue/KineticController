#include "FastLED.h"
#include <ArduinoWebsockets.h>
using namespace websockets;

extern int tile_shortid;
extern int num_tiles;
extern uint8_t gHue;
extern int tile_longids[34][4];
extern int tile_connections[34][3];
extern CRGB leds[34];
extern int NUM_LEDS;
extern int speed;
extern uint8_t brightness;
extern uint8_t currentPaletteIndex;
extern const CRGBPalette16 palettes[];
extern WebsocketsClient client;