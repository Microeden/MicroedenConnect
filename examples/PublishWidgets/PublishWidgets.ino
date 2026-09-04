#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MicroedenConnect.h>
#include "microeden_secrets.h"

// This example demonstrates typed widgets and arbitrary JSON fields.
WiFiClientSecure net;
MicroedenConnect device;
Level temperature("temperature", device);
Slider brightness("brightness", device);
Led online("online", device);
Photo statusText("status", device);

// Publish at a fixed interval instead of on every loop iteration.
unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000;

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
  // Increase the MQTT packet limit when payloads contain many fields.
  device.setBufferSize(512);
}

void loop() {
  // Process MQTT traffic and reconnect when the connection is lost.
  device.run();

  if (!device.isConnected() || millis() - lastPublish < PUBLISH_INTERVAL) {
    return;
  }
  lastPublish = millis();

  // Each write contributes a field to the same MQTT JSON payload.
  temperature.write(23);
  brightness.write(75);
  online.write(true);
  statusText.write("Device is online");
  device.writeKeyWord("voltage", 3.30);
  device.writeKeyWord("alarm", false);

  device.send();
}
