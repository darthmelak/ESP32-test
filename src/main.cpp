#include <Arduino.h>
#include <WifiConfig.hpp>
#include <SerialHandler.hpp>
#include <arduino-timer.h>
#include "secrets.h"

#define PIN_LED 16

void serialCb(String);

bool debug = true;
WifiConfig wifiConfig(WIFI_SSID, WIFI_PASSWORD, "ESP32S2 Tester Hub", "s2-tester", AUTH_USER, AUTH_PASS, true, true, debug);
Timer<1> timer;
// put function declarations here:

void setup() {
  if (debug) { Serial.begin(115200); delay(10); }

  pinMode(PIN_LED, OUTPUT);

  timer.every(660, [](void*) -> bool {
    if (!wifiConfig.isWifiConnected()) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    } else {
      digitalWrite(PIN_LED, HIGH);
    }

    return true;
  });
  wifiConfig.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  handleSerial(debug, serialCb);
  wifiConfig.loop();
  timer.tick<void>();
  delay(1);
}

void serialCb(String buffer) {
  if (buffer == "s") {
    Serial.println("Wifi Status: " + String(WiFi.status()));
  }
}