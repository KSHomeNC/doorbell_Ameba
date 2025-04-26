#ifndef __LED_MANAGER__
#define __LED_MANAGER__

#define BLUE_LED_PIN 19
#define GREEN_LED_PIN 20
#define RED_LED_PIN 21


struct LEDPattern {
  int priority;
  //int patternDurationMs; 
  int pattern[3]; // Array to hold the pattern for Blue, Green, and Red LEDs
  int dutyCycle[3]; // Array to hold the duty cycle for Blue, Green, and Red LEDs
};


enum LEDColor {
  BLUE = 0,
  GREEN = 1,
  RED = 2
};

enum priority {
  LOW_PRIORITY = 0,
  MEDIUM_PRIORITY = 1,
  HIGH_PRIORITY = 2
};

class LEDManager {
  private:
    LEDPattern currentPattern;
    LEDPattern nextPattern;
    unsigned long previousMillis[3];
    bool ledState[3];
    bool isPattern=false; // true for pattern generation false study led
    int ledWdTimerMSec=0;
    unsigned long startMsec=0;
    
    void reloadLedWdTimerMSec();
    void ledWdHandler();

    void updateLEDPattern() ;
    void setLEDPattern(LEDPattern pattern) ;

  public:
    LEDManager() ;
    void begin() ;
    
    void setNextPattern(LEDPattern pattern);
    void setLEDState(int ledIndex, bool state);
    void setLEDStateAll(bool state);
    void loop();
};
#endif //__LED_MANAGER__