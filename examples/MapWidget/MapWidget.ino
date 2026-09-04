#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MicroedenConnect.h>
#include "microeden_secrets.h"

// This example publishes a fixed location every 30 seconds.
WiFiClientSecure net;
MicroedenConnect device;
Map position("position", device);

// Keep trying until the Nano ESP32 has joined the configured Wi-Fi network.
void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
    delay(5000);
  }
}

void setup() {
  connectWiFi();
  // The ESP32 overload configures the bundled MicroEden root CA automatically.
  device.begin(SECRET_DEVICE_ID, SECRET_DEVICE_TOKEN, net);
}

void loop() {
  device.run();

  if (device.isConnected()) {
    // Replace these coordinates with values obtained from your GPS module.
    position.write(41.902782, 12.496366);
    device.send();
    delay(30000);
  }
}
