#include <Arduino.h>
#include <WifiConfig.hpp>
#include <SerialHandler.hpp>
#include <arduino-timer.h>
#include "secrets.h"

#ifndef PIN_LED
#define PIN_LED 16
#endif

#ifdef RGB_LED
#include <FastLED.h>

#define NUM_LEDS 1
#define DATA_PIN 48

CRGB leds[NUM_LEDS];
bool light = true;
#endif

void serialCb(String);
void ScanWiFi();

bool debug = true;
bool runwifi = true;
WifiConfig wifiConfig(WIFI_SSID, WIFI_PASSWORD, "ESP32S2 Tester Hub", "s2-tester", AUTH_USER, AUTH_PASS, true, true, debug);
Timer<2> timer;
// put function declarations here:

void setup() {
  if (debug) { Serial.begin(115200); delay(10); }

  pinMode(PIN_LED, OUTPUT);

  timer.every(660, [](void*) -> bool {
    if (!wifiConfig.isWifiConnected()) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    } else {
      digitalWrite(PIN_LED, LOW);
    }

    return true;
  });
  wifiConfig.begin();

  #ifdef RGB_LED
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    timer.every(1000, [](void*) -> bool {
      leds[0] = light ? CRGB::Red4 : CRGB::Black;
      FastLED.show();
      light = !light;

      return true;
    });
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
  handleSerial(debug, serialCb);
  if (runwifi) wifiConfig.loop();
  timer.tick<void>();
  delay(1);
}

void serialCb(String buffer) {
  if (buffer == "st") {
    Serial.println("Wifi Status: " + String(WiFi.status()));
  } else if (buffer == "w") {
    runwifi = !runwifi;
    Serial.println("Wifi " + runwifi ? "running" : "paused");
  } else if (buffer == "sc") {
    if (runwifi) runwifi = false;
    WiFi.disconnect();
    ScanWiFi();
  } else if (buffer == "s") {
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  }
}

void ScanWiFi() {
  Serial.println("Scan start");
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4" PRIi32, WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2" PRIi32, WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  Serial.println("-------------------------------------");
}