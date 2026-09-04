#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MicroedenConnect.h>
#include "microeden_secrets.h"

// This example maps MicroEden commands to the Nano ESP32 built-in LED.
WiFiClientSecure net;
MicroedenConnect device;
Led builtInLed("led", device);

// Keep trying until the Nano ESP32 has joined the configured Wi-Fi network.
void connectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
    delay(5000);
  }
}

// Publish the new state so the platform reflects the physical LED.
void publishLedState(bool isOn) {
  builtInLed.write(isOn);
  device.send();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  connectWiFi();
  // The ESP32 overload configures the bundled MicroEden root CA automatically.
  device.begin(SECRET_DEVICE_ID, SECRET_DEVICE_TOKEN, net);
}

void loop() {
  // run() also delivers inbound MQTT messages to the command handler.
  device.run();

  if (!device.isConnected()) {
    return;
  }

  // Commands are read from the default "content" field of an inbound payload.
  if (device.onCommand("ledon")) {
    digitalWrite(LED_BUILTIN, HIGH);
    publishLedState(true);
  }

  if (device.onCommand("ledoff")) {
    digitalWrite(LED_BUILTIN, LOW);
    publishLedState(false);
  }
}
