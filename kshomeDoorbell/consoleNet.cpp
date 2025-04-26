#include <WiFiUdp.h>
#include <arduino.h>
#include "WDT.h"
#include "consoleNet.h"

WiFiUDP Udp;
IPAddress remoteIp;
int remotePort;

consoleNet::consoleNet(consoleNetCallback cb)
{
  callBack=cb;
  Udp.begin(udpPort); 
  isStarted=true;
}
consoleNet::consoleNet(consoleNetCallback cb, int uPort)
{
  callBack=cb;
  udpPort = uPort;
  Udp.begin(udpPort);
  isStarted=true; 
}
void consoleNet::setCallback (consoleNetCallback cb)
{
  callBack=cb;
  Udp.begin(udpPort);
  isStarted=true; 
}
void consoleNet::consoleNetHandler(void)
{
  if (isStarted){
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        remoteIp = Udp.remoteIP();        
        Serial.print(remoteIp);
        remotePort = Udp.remotePort();
        Serial.print(", port ");
        Serial.println(remotePort);

        // read the packet into packetBufffer
        int len = Udp.read(rxBuff, 255);
        if (len > 0) {
          callBack(String(rxBuff));      
        }
        
        Serial.println("Contents:");
        Serial.println(rxBuff);    
    }
  }
}

void consoleNet:: consoleNetLoop(){
  consoleNetHandler();
}
int consoleNet:: sendResponse(char* res, int len){
  int retVal =-1;
  if(isStarted){
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(remoteIp, remotePort);
    if(Udp.write(res, len) > 0){
      retVal = 0;  
      Serial.print("remoteIP and Port     ");
      Serial.print(remoteIp);
      Serial.print("  :  ");
      Serial.println(remotePort);
      Serial.print("buffer size to be sent : ");
      Serial.println(len);
    }
    Udp.endPacket();    
  }
  return retVal;
}
