
#include <arduino.h>
#include "buttonManager.h"

void ButtonManager:: begin() {
  pinMode(buttonPin, INPUT);
}

bool ButtonManager:: isButtonPressed() {
  if (digitalRead(buttonPin) == HIGH && !buttonPressed) {
	buttonPressed = true;
	buttonPressTime = millis();
	return true;
  }
  if (buttonPressed && millis() - buttonPressTime >= 10000) {
	buttonPressed = false;
  }
  return false;
}
