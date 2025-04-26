#include <arduino.h>
#include "ledManager.h"

LEDManager::LEDManager() {
  currentPattern = {0, {LOW, LOW, LOW}, {0, 0, 0}};
  nextPattern = {0, {LOW, LOW, LOW}, {0, 0, 0}};
  for (int i = 0; i < 3; i++) {
	previousMillis[i] = 0;
	ledState[i] = LOW;
  }
}

void LEDManager:: begin() {
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
}

void LEDManager::setLEDPattern(LEDPattern pattern) {
  if(isPattern){
    for (int i = 0; i < 3; i++) {
      if (pattern.pattern[i] == HIGH) {
        unsigned long currentMillis = millis();
        if(currentMillis - previousMillis[i] >= pattern.dutyCycle[i]) {
        previousMillis[i] = currentMillis;
        ledState[i] = !ledState[i];
        digitalWrite(i == 0 ? BLUE_LED_PIN : (i == 1 ? GREEN_LED_PIN : RED_LED_PIN), ledState[i]);
        }
      } else {
        digitalWrite(i == 0 ? BLUE_LED_PIN : (i == 1 ? GREEN_LED_PIN : RED_LED_PIN), LOW);
      }
    }
  }
}

void LEDManager:: reloadLedWdTimerMSec(){
  ledWdTimerMSec = 10000; // deafult 10 sec 
  startMsec = millis();
}
void LEDManager:: ledWdHandler(){
  if(ledWdTimerMSec>0)
  {
    ledWdTimerMSec-=1; //(ledWdTimerMSec-(millis()-startMsec));
  }else{
    setLEDStateAll(false);
    ledWdTimerMSec=0;
  }
}
void LEDManager:: updateLEDPattern() {
  if (nextPattern.priority > currentPattern.priority ) {
    currentPattern = nextPattern;
    setLEDPattern(currentPattern);    
  }
}

void LEDManager::setNextPattern(LEDPattern pattern) {
  nextPattern = pattern;
  isPattern = true;
  reloadLedWdTimerMSec();
}

void LEDManager::loop() {
  updateLEDPattern();
  setLEDPattern(currentPattern);
  ledWdHandler();
}
void LEDManager::setLEDState(int ledIndex, bool state) {
  isPattern =false;
  if (ledIndex >= 0 && ledIndex < 3) {
	ledState[ledIndex] = state;
	digitalWrite(ledIndex == 0 ? BLUE_LED_PIN : (ledIndex == 1 ? GREEN_LED_PIN : RED_LED_PIN), state);
  }
  reloadLedWdTimerMSec();
}
void LEDManager::setLEDStateAll(bool state) {
  isPattern =false;
  for (int i = 0; i < 3; i++) {
	setLEDState(i, state);
  }
  if(state){
    reloadLedWdTimerMSec();
  }
}