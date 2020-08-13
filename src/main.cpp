/*
       _            _   _   ___            _             _ 
  /\ /(_)_ __   ___| |_(_) / __\___  _ __ | |_ _ __ ___ | |
 / //_/ | '_ \ / _ \ __| |/ /  / _ \| '_ \| __| '__/ _ \| |
/ __ \| | | | |  __/ |_| / /__| (_) | | | | |_| | | (_) | |
\/  \/|_|_| |_|\___|\__|_\____/\___/|_| |_|\__|_|  \___/|_|
                                                           
ESP32 based controller for the EFM8 based Kinetic Light Triangles

Licensed under the GNU GPL v3.0 license.

Copyright 2020, Aiden Vigue

*/

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "tileprotocol.h"
#include <FastLED.h>
#include "globals.h"
#include "effects.h"
#include "palettes.h"
#include <WebServer.h>
#include "secrets.h"
#include <Espalexa.h>

CRGB leds[34];

//Make a "secrets.h" file with the following
/*
#define WIFI_SSID "YOUR NETWORK NAME"
#define WIFI_PSK "PASSWORD"
*/
 
//async 1 wire pins
#define RXD2 16
#define TXD2 17 //reverse diode & pullup to 3v3
#define DEBUG
#define DEVICE_NAME "Picoleaf"

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

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
uint8_t brightness = 127;
uint8_t lastBrightness;
uint8_t currentPatternIndex = 0;
uint8_t currentPaletteIndex = 0;
uint8_t gCurrentPaletteNumber = 0;
int speed = 127;
bool lightsOn = true;
CRGBPalette16 gCurrentPalette(CRGB::Black);
CRGBPalette16 gTargetPalette(gGradientPalettes[0]);
extern const TProgmemRGBGradientPalettePtr gGradientPalettes[];
CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

Espalexa espalexa;
EspalexaDevice* AlexaPowerStateDevice;

void AlexaPowerStateCallback(uint8_t b);

void colorWaves()
{
    colorwaves(leds, num_tiles * 3, gCurrentPalette);
}

PatternAndNameList patterns = {
  {pride, "Pride"},
  {juggle, "Juggle"},
  {bpm, "BPM"},
  {confetti, "Confetti"},
  {colorWaves, "Color Waves"},
  {randomPaletteFades, "Random Palette Fades"}
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

void setBrightness(uint8_t b, bool fromAlexa = 0) {
  brightness = b;
  EEPROM.write(0, b);
  EEPROM.commit();
  if(fromAlexa == 0) {
    AlexaPowerStateDevice->setValue(brightness);
  }
}

void setPattern(uint8_t value)
{
    if (value >= patternCount)
        value = patternCount - 1;

    currentPatternIndex = value;
    EEPROM.write(1, currentPatternIndex);
    EEPROM.commit();
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
    EEPROM.write(8, currentPaletteIndex);
    EEPROM.commit();
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

void loadSettings()
{
    brightness = EEPROM.read(0);
    currentPatternIndex = EEPROM.read(1);
    if (currentPatternIndex < 0)
        currentPatternIndex = 0;
    else if (currentPatternIndex >= patternCount)
        currentPatternIndex = patternCount - 1;
    
    currentPaletteIndex = EEPROM.read(8);
    if (currentPaletteIndex < 0)
        currentPaletteIndex = 0;
    else if (currentPaletteIndex >= paletteCount)
        currentPaletteIndex = paletteCount - 1;
    speed = EEPROM.read(9);
}

void setup() {
  delay(1000);
  AlexaPowerStateDevice = new EspalexaDevice(DEVICE_NAME, AlexaPowerStateCallback);
  espalexa.addDevice(AlexaPowerStateDevice);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2100);
  EEPROM.begin(20);
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
  loadSettings();
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
      break;
    }
    AlexaPowerStateDevice->setState(lightsOn);
    AlexaPowerStateDevice->setValue(brightness);
  }

  //Turn all discovered tiles off.
  for(int i = 0; i < num_tiles; i++) {
    setColor(i+1, 0, 0, 0);
  }

  //Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PSK);

  while(WiFi.status() != WL_CONNECTED) {
  }

  Serial.println(WiFi.localIP());

  setColor(0xFF, 0, 0, 0);
  server.on("/pattern", []() {
    String pattern = server.arg("value");
    setPattern(pattern.toInt());
    server.send(200, "text/plain", "ACK");
  });
  server.on("/palette", []() {
    String pattern = server.arg("value");
    setPalette(pattern.toInt());
    server.send(200, "text/plain", "ACK");
  });
  server.on("/power", []() {
    String pattern = server.arg("value");
    if(pattern.toInt() == 1) {
      if(lastBrightness == 0) {
        lastBrightness == 255;
      }
      setBrightness(lastBrightness, false);
    } else {
      lastBrightness = brightness;
      setBrightness(0, false);
    }
    server.send(200, "text/plain", "ACK");
  });
  server.on("/brightness", []() {
    String pattern = server.arg("value");
    setBrightness(pattern.toInt(), false);
    server.send(200, "text/plain", "ACK");
  });
  server.onNotFound([](){
    if (!espalexa.handleAlexaApiCall(server.uri(),server.arg(0)))
    {
      server.send(404, "text/plain", "Not found");
    }
  });
  espalexa.begin(&server);
}

void loop() {
  espalexa.loop();
  //server.handleClient();
  patterns[currentPatternIndex].pattern();
  EVERY_N_SECONDS(20) {
      gCurrentPaletteNumber = addmod8(gCurrentPaletteNumber, 1, gGradientPaletteCount);
      gTargetPalette = gGradientPalettes[gCurrentPaletteNumber];
  }

  EVERY_N_MILLISECONDS(40) {
      // slowly blend the current palette to the next
      nblendPaletteTowardPalette(gCurrentPalette, gTargetPalette, 8);
      gHue++;  // slowly cycle the "base color" through the rainbow
  }
  display();
  delay(1000 / 120);
}

void AlexaPowerStateCallback(uint8_t b) {
  setBrightness(b, true);
}

