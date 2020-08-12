#include <Arduino.h>
char send_buf[10];
char recv_buf[10];
int tries = 0;

void waitForResponse(int bytesSent, char * recv_buf) {
    char recv;
    int recvPos = 0;
    int tm = 0;
    while(Serial2.read() != 0x80) {
        tm++;
        int tmx = 0; 
        if(tm == 10) {
            recv_buf[0] = 0x40;
            return;
        }
        while(!Serial2.available()) {
            tmx++;
            delay(1);
            if(tmx == 30) {
                break;
            }
        }
    }
    while(Serial2.available()) {
        recv = Serial2.read();
        //Serial.printf("Recieved: %X\n", recv);
        if(recv != 0xA0) {
            recv_buf[recvPos] = recv;
            recvPos++;
        } else {
            while(Serial2.available()) {
                Serial2.read(); //flush buffers.
            }
            break;
        }
    }
}

bool getLongIDOfSelectedTile(char * long_id) {
    int tries;
    while(true) {
        Serial2.write(0x03);
        Serial2.write(0x0A);
        waitForResponse(4, recv_buf);
        if(recv_buf[0] == 0x40) {
            tries++;
            if(tries > 10) {
                tries = 0;
                break;
            }
        } else {
            Serial.printf("Got long ID of tile: %X%X%X%X\n", recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3]);
            long_id[0] = recv_buf[0];
            long_id[1] = recv_buf[1];
            long_id[2] = recv_buf[2];
            long_id[3] = recv_buf[3];
            return true;
        }
    }
    return false; 
}

void setShortID(char * longid, char shortid) {
    int tries;
    while(true) {
        Serial.printf("Setting short id of %X%X%X%X to %X\n", longid[0], longid[1], longid[2], longid[3], shortid);
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
            tries++;
            if(tries == 3) {
                break;
            }
        } else {
            break;
        }
    }
}

void setEdge(char shortid, char bitmask) {
    int tries;
    while(true) {
        Serial.printf("Setting tile %X edge bitmask to %X\n", shortid, bitmask);
        Serial2.write(0x02);
        Serial2.write(shortid);
        Serial2.write(bitmask);
        Serial2.write(0x0A);
        waitForResponse(4, recv_buf);
        if(recv_buf[0] == 0x40) {
            tries++;
            if(tries == 3) {
                return;
            }
        } else {
            break;
        }
    }
}

void setColor(char tileid, char r, char g, char b) {
    int tries;
    while(true) {
        Serial.printf("Setting tile %X color to #%X%X%X\n", tileid, r,g,b);
        Serial2.write(0x08);
        Serial2.write(tileid);
        Serial2.write(r);
        Serial2.write(g);
        Serial2.write(b);
        Serial2.write(0x0A);
        waitForResponse(4, recv_buf);
        if(recv_buf[0] == 0x40) {
            tries++;
            if(tries == 3) {
                return;
            }
        } else {
            break;
        }
    }
}