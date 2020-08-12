bool getLongIDOfSelectedTile(char * long_id);
void setShortID(char longid[3], char shortid);
void setEdge(char shortid, char bitmask);
void setColor(char tileid, char r, char g, char b);
void waitForResponse(char * recv_buf);