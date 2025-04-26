#include "OTA.h"
#include"otaManager.h"

OTA ota;

bool OTA_Manager:: startOTA(){
  bool retType = false;
  if(status){
   ota.start_OTA_threads(port, serverIp);
   retType = true;
  }
  return retType;
}

void OTA_Manager::printOTAServerInfo(){
  Serial.print(" server Port: ");
  Serial.println(port);
  Serial.print(" server IP: ");
  Serial.println(serverIp);
}