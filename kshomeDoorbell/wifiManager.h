#ifndef __WIFI_MANAGER__
#define __WIFI_MANAGER__
#include <WiFi.h>
class WiFiManager {
  private:
    const char* ssid = "subhSpec";
    const char* password = "pawan@158";
    //const char* ssid = "SE_RTC_05_2.4G";
    //const char* password = "9197466616";
  public:
    void begin() ;
    void setupWiFi() ;
};

#endif //__WIFI_MANAGER__