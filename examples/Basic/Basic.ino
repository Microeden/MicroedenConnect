#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MicroedenConnect.h>
#include "microeden_secrets.h"

// This example sends one simple text field every five seconds.
WiFiClientSecure net;
MicroedenConnect device;

// Keep trying until the Nano ESP32 has joined the configured Wi-Fi network.
void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
    delay(5000);
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  // On ESP32, begin() automatically installs the MicroEden root CA.
  device.begin(SECRET_DEVICE_ID, SECRET_DEVICE_TOKEN, net);
}

void loop() {
  // run() handles MQTT reconnects and incoming messages.
  device.run();

  if (device.isConnected()) {
    // Fields queued with writeKeyWord() are sent as one JSON payload.
    device.writeKeyWord("message", "Hello from Arduino");
    device.send();
    delay(5000);
  }
}
