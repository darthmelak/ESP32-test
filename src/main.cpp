#include <Arduino.h>
#include <WifiConfig.hpp>
#include <SerialHandler.hpp>
#include <arduino-timer.h>
#include "secrets.h"

#ifndef PIN_LED
#define PIN_LED 16
#endif

#ifdef BOARD_S3
#define COM_PORT Serial
#else
#define COM_PORT Serial
#endif

void serialCb(String);
void ScanWiFi();

bool debug = true;
bool runwifi = true;
WifiConfig wifiConfig(WIFI_SSID, WIFI_PASSWORD, "ESP32S2 Tester Hub", "s2-tester", AUTH_USER, AUTH_PASS, true, true, debug);
Timer<1> timer;
// put function declarations here:

void setup() {
  if (debug) { COM_PORT.begin(115200); delay(10); }

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
    COM_PORT.println("Wifi Status: " + String(WiFi.status()));
  } else if (buffer == "w") {
    runwifi = !runwifi;
    COM_PORT.println("Wifi " + runwifi ? "running" : "paused");
  } else if (buffer == "sc") {
    if (runwifi) runwifi = false;
    WiFi.disconnect();
    ScanWiFi();
  } else if (buffer == "s") {
    COM_PORT.printf("IP address: %s\n", WiFi.localIP().toString().c_str());
  }
}

void ScanWiFi() {
  COM_PORT.println("Scan start");
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  COM_PORT.println("Scan done");
  if (n == 0) {
    COM_PORT.println("no networks found");
  } else {
    COM_PORT.print(n);
    COM_PORT.println(" networks found");
    COM_PORT.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      COM_PORT.printf("%2d", i + 1);
      COM_PORT.print(" | ");
      COM_PORT.printf("%-32.32s", WiFi.SSID(i).c_str());
      COM_PORT.print(" | ");
      COM_PORT.printf("%4" PRIi32, WiFi.RSSI(i));
      COM_PORT.print(" | ");
      COM_PORT.printf("%2" PRIi32, WiFi.channel(i));
      COM_PORT.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            COM_PORT.print("open"); break;
        case WIFI_AUTH_WEP:             COM_PORT.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         COM_PORT.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        COM_PORT.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    COM_PORT.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: COM_PORT.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        COM_PORT.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   COM_PORT.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        COM_PORT.print("WAPI"); break;
        default:                        COM_PORT.print("unknown");
      }
      COM_PORT.println();
      delay(10);
    }
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  COM_PORT.println("-------------------------------------");
}