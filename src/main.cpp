#include <Arduino.h>
#include <OneButton.h>
#include "Adafruit_TinyUSB.h"

bool debug = true;
OneButton button(GPIO_NUM_23);

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD()
};
// put function declarations here:

void setup() {
  if (debug) { Serial.begin(115200); delay(10); }

  button.attachClick([]() {
    if (debug) Serial.println("!btn!");
  });
}

void loop() {
  // put your main code here, to run repeatedly:
  button.tick();
}

