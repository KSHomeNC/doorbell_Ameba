#ifndef __BUTTON_MANAGER__
#define __BUTTON_MANAGER__

#define BUTTON_PIN 22
class ButtonManager {
  private:
    bool buttonPressed;
    unsigned long buttonPressTime;
    int buttonPin;

  public:
    ButtonManager(int pin) : buttonPin(pin) {
      buttonPressed = false;
      buttonPressTime = 0;
    }

    void begin();

    bool isButtonPressed() ;
};

#endif //__BUTTON_MANAGER__