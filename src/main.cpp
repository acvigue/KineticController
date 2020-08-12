/**
 * EFM8 based Kinetic Light Tile controller.
 * Licensed under GNU GPL v3.
 * Aiden Vigue, 2020.
 */

#include <Arduino.h>
#include "tileprotocol.h"
 
//async 1 wire pins
#define RXD2 16
#define TXD2 17 //reverse diode & pullup to 3v3
#define DEBUG
//Max 35 tiles on one controller
int tile_shortid = 1;
int tile_longids[34][3]; // {0xB1, 0xB2, 0xB3, 0xB4} (Used to find circular connections)
int tile_connections[34][2]; // {0, <short_id>, <short_id>} (Array of what direction on tile corresponds to next tile)
char unexplored_tiles[34];
int unexplored_index = 1;
char dirToMask(int dir) {
  if(dir == 1) {
    return 0x01;
  } else if(dir == 2) {
    return 0x02;
  } else {
    return 0x04;
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial.available()) {} //wait for a byte from serial to start
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
  char longID[3];
  getLongIDOfSelectedTile(longID);
  digitalWrite(22, HIGH);
  tile_longids[tile_shortid-1][0] = longID[0];
  tile_longids[tile_shortid-1][1] = longID[1];
  tile_longids[tile_shortid-1][2] = longID[2];
  tile_longids[tile_shortid-1][3] = longID[3];
  setShortID(longID, tile_shortid);
  setColor(tile_shortid, 0,0,0);
  unexplored_tiles[0] = tile_shortid;
  tile_shortid++;
  while(true) {
    int currentlyExploring = unexplored_tiles[0];
    int currentShortID = tile_shortid;

    setEdge(currentlyExploring, 0x01);
    delay(10);
    if(getLongIDOfSelectedTile(longID) == true) {
      Serial.printf("Found tile %X%X%X%X attached on side A of tile %X\n", longID[0], longID[1], longID[2], longID[3], currentShortID);
      tile_longids[tile_shortid-1][0] = longID[0];
      tile_longids[tile_shortid-1][1] = longID[1];
      tile_longids[tile_shortid-1][2] = longID[2];
      tile_longids[tile_shortid-1][3] = longID[3];
      setShortID(longID, tile_shortid);
      setColor(tile_shortid, 0, 0, 0);
      unexplored_tiles[unexplored_index] = tile_shortid;
      unexplored_index++;
      tile_connections[currentShortID-1][0] = tile_shortid;
      tile_shortid++;
    }

    setEdge(currentlyExploring, 0x02);
    delay(10);
    if(getLongIDOfSelectedTile(longID) == true) {
      Serial.printf("Found tile %X%X%X%X attached on side B of tile %X\n", longID[0], longID[1], longID[2], longID[3], currentShortID);
      tile_longids[tile_shortid-1][0] = longID[0];
      tile_longids[tile_shortid-1][1] = longID[1];
      tile_longids[tile_shortid-1][2] = longID[2];
      tile_longids[tile_shortid-1][3] = longID[3];
      setShortID(longID, tile_shortid);
      setColor(tile_shortid, 0, 0, 0);
      unexplored_tiles[unexplored_index] = tile_shortid;
      unexplored_index++;
      tile_connections[currentShortID-1][1] = tile_shortid;
      tile_shortid++;
    }

    setEdge(currentlyExploring, 0x04);
    delay(10);
    if(getLongIDOfSelectedTile(longID) == true) {
      Serial.printf("Found tile %X%X%X%X attached on side C of tile %X\n", longID[0], longID[1], longID[2], longID[3], currentShortID);
      tile_longids[tile_shortid-1][0] = longID[0];
      tile_longids[tile_shortid-1][1] = longID[1];
      tile_longids[tile_shortid-1][2] = longID[2];
      tile_longids[tile_shortid-1][3] = longID[3];
      setShortID(longID, tile_shortid);
      setColor(tile_shortid, 0, 0, 0);
      unexplored_tiles[unexplored_index] = tile_shortid;
      unexplored_index++;
      tile_connections[currentShortID-1][2] = tile_shortid;
      tile_shortid++;
    }

    //Discovery loop
    if(strlen(unexplored_tiles) != 1) {
      char* unexplored_popping = unexplored_tiles + 1;
      memmove(unexplored_tiles, unexplored_popping, strlen(unexplored_popping) + 1);
      unexplored_index--;
    } else {
      //Last unexplored tile
      Serial.println("Discovery complete.");
      break;
    }
  }
}

void loop() {

}

