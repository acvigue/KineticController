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
char tile_shortid = 0x01;
char tile_longids[34][3];
 
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
  Serial.println("ESP32 Tile controller test");
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(22, OUTPUT); //DET pin on controller
  digitalWrite(22, HIGH);

  //Discovery
  char longID[3];
  getLongIDOfSelectedTile(longID);
  digitalWrite(22, LOW);
  setShortID(longID, 0x01);
  setColor(0x01, 0xFF, 0x00, 0x00);
  tile_shortid++;
}

void loop() {

}

