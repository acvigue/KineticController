void getLongIDOfSelectedTile(char * long_id);
bool setShortID(char longid[3], char shortid);
bool setEdge(char shortid, char bitmask);
bool setColor(char tileid, char r, char g, char b);
void waitForResponse(char * recv_buf);