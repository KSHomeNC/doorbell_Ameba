#include "IPAddress.h"
#ifndef __OTA_MANAGER__
#define __OTA_MANAGER__

#include <stdio.h>

// the Ameba board will ba a OTA HTTP server and UI tool will be a HTTP client
#define MAX_LEN_IP 17
class OTA_Manager{
  private:
    int port;
    char serverIp[MAX_LEN_IP];
    bool status;
        
  public: 

  OTA_Manager(){  
   
    status = false;
  }
  OTA_Manager(int serverPort, char* IPadd ){  
    port = serverPort;
    memcpy((void*)&serverIp[0], (void*) IPadd, MAX_LEN_IP);
    status = true;
  }
  bool startOTA();
  void printOTAServerInfo();
};

#endif //__OTA_MANAGER__