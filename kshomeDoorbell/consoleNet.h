#ifndef _CONSOL_NET_H___
#define _CONSOL_NET_H___

typedef void (*consoleNetCallback)(String command);

class consoleNet{
  private:
    int udpPort=1235;
    char rxBuff[256];
    consoleNetCallback callBack;
    bool isStarted= false;

    void consoleNetHandler(void);
  public:
    consoleNet(){callBack = (consoleNetCallback)0;}
    consoleNet(consoleNetCallback cb);
    consoleNet(consoleNetCallback cb, int udpPort);

    void setCallback (consoleNetCallback cb);
    int sendResponse(char* res, int len);
    void consoleNetLoop();
};





#endif //_CONSOL_NET_H___
