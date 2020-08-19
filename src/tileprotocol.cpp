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
            if(tries > 3) {
                tries = 0;
                break;
            }
        } else {
            Serial.printf("Got long ID of tile: %2X%2X%2X%2X\n", recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3]);
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
        Serial.printf("Setting short id of %2X%2X%2X%2X to %X\n", longid[0], longid[1], longid[2], longid[3], shortid);
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
        Serial.printf("Setting tile %2X edge bitmask to %2X\n", shortid, bitmask);
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

void setColor(char tileid, char r1, char g1, char b1, char r2, char g2, char b2, char r3, char g3, char b3) {
    if(r1 == 0x0A) {
        r1 = 0x0B;
    }
    if(g1 == 0x0A) {
        g1 = 0x0B;
    }
    if(b1 == 0x0A) {
        b1 = 0x0B;
    }
    if(r1 == 0x80) {
        r1 = 0x81;
    }
    if(g1 == 0x80) {
        g1 = 0x81;
    }
    if(b1 == 0x80) {
        b1 = 0x81;
    }
    if(r2 == 0x0A) {
        r2 = 0x0B;
    }
    if(g2 == 0x0A) {
        g2 = 0x0B;
    }
    if(b2 == 0x0A) {
        b2 = 0x0B;
    }
    if(r2 == 0x80) {
        r2 = 0x81;
    }
    if(g2 == 0x80) {
        g2 = 0x81;
    }
    if(b2 == 0x80) {
        b2 = 0x81;
    }
    if(r3 == 0x0A) {
        r3 = 0x0B;
    }
    if(g3 == 0x0A) {
        g3 = 0x0B;
    }
    if(b3 == 0x0A) {
        b3 = 0x0B;
    }
    if(r3 == 0x80) {
        r3 = 0x81;
    }
    if(g3 == 0x80) {
        g3 = 0x81;
    }
    if(b3 == 0x80) {
        b3 = 0x81;
    }
    
    //Serial.printf("Setting tile %X color to #%2X%2X%2X\n", tileid, r,g,b);
    Serial2.write(0x08);
    Serial2.write(tileid);
    Serial2.write(r1);
    Serial2.write(g1);
    Serial2.write(b1);
    Serial2.write(r2);
    Serial2.write(g2);
    Serial2.write(b2);
    Serial2.write(r3);
    Serial2.write(g3);
    Serial2.write(b3);
    Serial2.write(0x0A);
}