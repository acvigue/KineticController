/**
 * EFM8 based Kinetic Light Tile controller.
 * Licensed under GNU GPL v3.
 * Aiden Vigue, 2020.
 */

#include <Arduino.h>
#include <WiFi.h>
#include "tileprotocol.h"
#include <FastLED.h>
#include "globals.h"
#include "effects.h"
#include "palettes.h"
#include <WebServer.h>

CRGB leds[34];
 
//async 1 wire pins
#define RXD2 16
#define TXD2 17 //reverse diode & pullup to 3v3
#define DEBUG

#define WIFI_SSID "tarheels"
#define WIFI_PSK "ILuzFJ522F9HaSqEDyU0pjA4aGG2pZJL"

//Max 35 tiles on one controller
int tile_shortid = 1;
int num_tiles;
int tile_longids[34][4]; // {0xB1, 0xB2, 0xB3, 0xB4} (Used to find circular connections)
int tile_connections[34][3]; // {0, <short_id>, <short_id>} (Array of what direction on tile corresponds to next tile)
char unexplored_tiles[34];
int unexplored_index = 1;
int NUM_LEDS;

WebServer server(80);
typedef void(*Pattern)();
typedef Pattern PatternList[];
typedef struct {
    Pattern pattern;
    String name;
} PatternAndName;
typedef PatternAndName PatternAndNameList[];

int currentPatternIndex = 0;
uint8_t currentPaletteIndex = 0;
uint8_t gCurrentPaletteNumber = 0;
int speed = 127;
CRGBPalette16 gCurrentPalette(CRGB::Black);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

uint8_t currentPatternIndex = 2; // Index number of which pattern is current
uint8_t autoplay = 0;

uint8_t autoplayDuration = 10;
unsigned long autoPlayTimeout = 0;

PatternAndNameList patterns = {
  {pride, "Pride"},
  {juggle, "Juggle"},
  {bpm, "BPM"},

};

const uint8_t patternCount = ARRAY_SIZE(patterns);

typedef struct {
    CRGBPalette16 palette;
    String name;
} PaletteAndName;
typedef PaletteAndName PaletteAndNameList[];

const CRGBPalette16 palettes[] = {
  RainbowColors_p,
  RainbowStripeColors_p,
  CloudColors_p,
  LavaColors_p,
  OceanColors_p,
  ForestColors_p,
  PartyColors_p,
  HeatColors_p
};

const uint8_t paletteCount = ARRAY_SIZE(palettes);

const String paletteNames[paletteCount] = {
  "Rainbow",
  "Rainbow Stripe",
  "Cloud",
  "Lava",
  "Ocean",
  "Forest",
  "Party",
  "Heat",
};

void setPattern(uint8_t value)
{
    if (value >= patternCount)
        value = patternCount - 1;

    currentPatternIndex = value;
}


void setPatternName(String name)
{
    for (uint8_t i = 0; i < patternCount; i++) {
        if (patterns[i].name == name) {
            setPattern(i);
            break;
        }
    }
}

void setPalette(uint8_t value)
{
    if (value >= paletteCount)
        value = paletteCount - 1;

    currentPaletteIndex = value;
}

void setPaletteName(String name)
{
    for (uint8_t i = 0; i < paletteCount; i++) {
        if (paletteNames[i] == name) {
            setPalette(i);
            break;
        }
    }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial2.write(0xFF);
  Serial2.write(0x0A);
  delay(50);
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
  char longID[4];
  getLongIDOfSelectedTile(longID);
  digitalWrite(22, HIGH);
  tile_longids[tile_shortid-1][0] = longID[0];
  tile_longids[tile_shortid-1][1] = longID[1];
  tile_longids[tile_shortid-1][2] = longID[2];
  tile_longids[tile_shortid-1][3] = longID[3];
  setShortID(longID, tile_shortid);
  unexplored_tiles[0] = tile_shortid;
  tile_shortid++;
  while(true) {
    int currentlyExploring = unexplored_tiles[0];
    setEdge(currentlyExploring, 0x01);
    delay(50);
    if(getLongIDOfSelectedTile(longID) == true) {
      Serial.printf("Found tile %X%X%X%X attached on side A of tile %i\n", longID[0], longID[1], longID[2], longID[3], currentlyExploring);
      tile_longids[tile_shortid-1][0] = longID[0];
      tile_longids[tile_shortid-1][1] = longID[1];
      tile_longids[tile_shortid-1][2] = longID[2];
      tile_longids[tile_shortid-1][3] = longID[3];
      setShortID(longID, tile_shortid);
      unexplored_tiles[unexplored_index] = tile_shortid;
      unexplored_index++;
      tile_connections[currentlyExploring-1][0] = tile_shortid;
      tile_shortid++;
    }

    setEdge(currentlyExploring, 0x02);
    delay(50);
    if(getLongIDOfSelectedTile(longID) == true) {
      Serial.printf("Found tile %X%X%X%X attached on side B of tile %i\n", longID[0], longID[1], longID[2], longID[3], currentlyExploring);
      tile_longids[tile_shortid-1][0] = longID[0];
      tile_longids[tile_shortid-1][1] = longID[1];
      tile_longids[tile_shortid-1][2] = longID[2];
      tile_longids[tile_shortid-1][3] = longID[3];
      setShortID(longID, tile_shortid);
      unexplored_tiles[unexplored_index] = tile_shortid;
      unexplored_index++;
      tile_connections[currentlyExploring-1][1] = tile_shortid;
      tile_shortid++;
    }

    setEdge(currentlyExploring, 0x04);
    delay(50);
    if(getLongIDOfSelectedTile(longID) == true) {
      Serial.printf("Found tile %X%X%X%X attached on side C of tile %X\n", longID[0], longID[1], longID[2], longID[3], currentlyExploring);
      tile_longids[tile_shortid-1][0] = longID[0];
      tile_longids[tile_shortid-1][1] = longID[1];
      tile_longids[tile_shortid-1][2] = longID[2];
      tile_longids[tile_shortid-1][3] = longID[3];
      setShortID(longID, tile_shortid);
      unexplored_tiles[unexplored_index] = tile_shortid;
      unexplored_index++;
      tile_connections[currentlyExploring-1][2] = tile_shortid;
      tile_shortid++;
    }

    if(unexplored_tiles[1] != 0) {
      for(int i=0;i<34;i++){
        if(i <= 33) {
          unexplored_tiles[i]=unexplored_tiles[i+1];
        } else {
          unexplored_tiles[i]=0;
        }
      }
      unexplored_index--;
    } else {
      //Last unexplored tile
      num_tiles = tile_shortid - 1;
      NUM_LEDS = num_tiles;
      Serial.println("Discovery complete.");
      break;
    }
  }

  //Turn all discovered tiles off.
  for(int i = 0; i < num_tiles; i++) {
    setColor(i+1, 0, 0, 0);
  }

  //Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  while(WiFi.status() != WL_CONNECTED) {
    if(WiFi.status() == WL_CONNECT_FAILED) {
      //connection failed.
      while(true) {
        //pulse all tiles red
        float val = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
        setColor(1, val, 0, 0);
        delay(15);
      }
    }

    //pulse tiles blue
    float val = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
    setColor(1, 0, 0, val);
    delay(15);
  }

  setColor(0xFF, 0, 0, 0);

  //Setup web server
  server.on("/setPattern", []() {
    String value = webServer.arg("value");
  });
  server.begin();
}

void loop() {
  server.handleClient();
  //patterns[currentPatternIndex].pattern();
  display();
}

