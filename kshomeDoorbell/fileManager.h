#ifndef __FILE_MANAGER__
#define __FILE_MANAGER__
#include "Arduino.h"

void fsBegin();
int fileRead(String fname, unsigned char* buff, int size);
int fileWrite( String fname, unsigned char* buff, int len);
bool fileRemove( String fname);
String getRootPath();
int getDirList(char *rxList, int size);

#endif //__FILE_MANAGER__