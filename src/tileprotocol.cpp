#include <Arduino.h>
char send_buf[10];
char recv_buf[10];
int tries = 0;

void waitForResponse(int bytesSent, char * recv_buf) {
    char recv;
    int recvPos = 0;
    recv_buf[0] = 0;
    bool shouldRead = false;
    int tm = 0;
    while(!Serial2.available()) {delay(50); tm++; if(tm == 3) {
        recv_buf[0] = 0x40;
        return;
    }}
    while(Serial2.available()) {
        recv = Serial2.read();
        Serial.println(recv, HEX);
        if(recv == 0x80) {
            shouldRead = true;
        }
        if(shouldRead == true) {
            if(recv != 0xA0) {
                Serial.print("Reading: ");
                Serial.println(recv, HEX);
                recv_buf[recvPos] = recv;
                recvPos++;
            } else {
                break;
            }
        }
    }
}

void getLongIDOfSelectedTile(char * long_id) {
    Serial2.write(0x03);
    Serial2.write(0x0A);
    waitForResponse(4, recv_buf);
    if(recv_buf[0] == 0x80) {
        //tile responded
        Serial.printf("Got long ID of tile: %X%X%X%X\n", recv_buf[1], recv_buf[2], recv_buf[3], recv_buf[4]);
        long_id[0] = recv_buf[1];
        long_id[1] = recv_buf[2];
        long_id[2] = recv_buf[3];
        long_id[3] = recv_buf[4];
    } else {
        tries++;
        if(tries > 6) {
            tries = 0;
            return;
        }
        delay(10);
        getLongIDOfSelectedTile(long_id);
    }
}

void setShortID(char longid[3], char shortid) {
    Serial.printf("Setting short id of %X%X%X%X to %X", longid[0], longid[1], longid[2], longid[3], shortid);
    Serial2.write(0x01);
    Serial2.write(0xFF);
    Serial2.write(longid[0]);
    Serial2.write(longid[1]);
    Serial2.write(longid[2]);
    Serial2.write(longid[3]);
    Serial2.write(shortid);
    Serial2.write(0x0A);
    waitForResponse(4, recv_buf);
    if(recv_buf[0] == 0x40) {
        delay(10);
        setShortID(longid, shortid);
    }
}

void setEdge(char shortid, char bitmask) {
    Serial.printf("Setting tile %X edge bitmask to %X\n", shortid, bitmask);
    Serial2.write(0x02);
    Serial2.write(shortid);
    Serial2.write(bitmask);
    Serial2.write(0x0A);

    waitForResponse(4, recv_buf);
    if(recv_buf[0] == 0x40) {
        Serial.println("Comm failed");
        delay(10);
        setEdge(shortid, bitmask);
    }
}

void setColor(char tileid, char r, char g, char b) {
    Serial2.write(0x08);
    Serial2.write(tileid);
    Serial2.write(r);
    Serial2.write(g);
    Serial2.write(b);
    Serial2.write(0x0A);
    waitForResponse(4, recv_buf);
    if(recv_buf[0] == 0x40) {
        delay(10);
        setColor(tileid, r, g, b);
    }
}