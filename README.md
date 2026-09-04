# MicroedenConnect

Arduino library for connecting Arduino-compatible boards to the MicroEden
platform over an MQTT connection.

It is compatible with any board and network stack that provides an Arduino
`Client` implementation. Common compatible boards include:

* Arduino Nano ESP32
* ESP32 Dev Module and other ESP32 boards
* Arduino Nano 33 IoT
* Arduino Nano RP2040 Connect
* Arduino MKR WiFi 1010
* Arduino UNO WiFi Rev2
* Arduino UNO R4 WiFi
* Arduino UNO Q, when used with an Arduino-compatible `Client` implementation
* Wi-Fi-enabled Portenta and Opta variants, using their board-specific client
* Ethernet boards, using a TLS-capable Ethernet client
* GSM/LTE boards, using a TLS-capable cellular client

## Installation

Install **MicroedenConnect** from the Arduino IDE Library Manager. The library
also depends on [PubSubClient](https://github.com/knolleary/pubsubclient) and
[ArduinoJson](https://arduinojson.org/); the IDE can install these dependencies
automatically.

In the Arduino IDE, select your board and install its board platform and network
library from Boards Manager. Use the ESP32 platform by Espressif for ESP32 boards,
**Arduino SAMD Boards** for the Nano 33 IoT, **Arduino Mbed OS Nano Boards** for
the Nano RP2040 Connect, and **Arduino UNO R4 Boards** for the UNO R4 WiFi.
Install **WiFiNINA** when it is the network library supplied by the selected board
(for example, Nano 33 IoT, Nano RP2040 Connect, MKR WiFi 1010, or UNO WiFi Rev2).
For the UNO Q, install the board support package recommended by Arduino; this
library applies to sketches running on its Arduino-programmable MCU when a
compatible `Client` implementation is available.

## First use

Open one of the examples from `File > Examples > MicroedenConnect`. In the
example's folder, edit `microeden_secrets.h` and replace its placeholders with
your Wi-Fi and MicroEden device credentials. Keep this file private and never
share a real token or password.

The ESP32 examples use `WiFiClientSecure`. When that client type is passed to
`device.begin(...)`, MicroedenConnect automatically configures the public
Let's Encrypt `ISRG Root X1` CA used by `microeden.io`. This applies to the Nano
ESP32, ESP32 Dev Module, and other boards using the Arduino ESP32 core.

The Nano 33 IoT, Nano RP2040 Connect, MKR WiFi 1010, and UNO WiFi Rev2 commonly
use `WiFiSSLClient` from WiFiNINA. The UNO R4 WiFi uses the secure client supplied
by WiFiS3. Pass either client to the generic `device.begin(..., Client&)` overload:

```cpp
#include <WiFiNINA.h>
#include <WiFiSSLClient.h>

WiFiSSLClient net;
MicroedenConnect device;

// After Wi-Fi has connected:
device.begin(SECRET_DEVICE_ID, SECRET_DEVICE_TOKEN, net);
```

The ESP32 clock must be synchronized before the first TLS connection:

```cpp
#include <time.h>

configTime(0, 0, "pool.ntp.org", "time.nist.gov");
while (time(nullptr) < 1700000000) {
  delay(500);
}
```

The MQTT broker host is `microeden.io` on port `8243`.

## Minimal example

```cpp
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MicroedenConnect.h>
#include "microeden_secrets.h"

WiFiClientSecure net;
MicroedenConnect device;

void setup() {
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1700000000) {
    delay(500);
  }

  // The ESP32 overload automatically configures the MicroEden root CA.
  device.begin(SECRET_DEVICE_ID, SECRET_DEVICE_TOKEN, net);
}

void loop() {
  device.run();

  if (device.isConnected()) {
    device.writeKeyWord("message", "Hello from Arduino");
    device.send();
    delay(5000);
  }
}
```

## API overview

* `device.run()` keeps MQTT connected and processes inbound messages. Call it
  frequently from `loop()`.
* `device.writeKeyWord(key, value)` adds a field to the next JSON payload.
* `device.send()` publishes the queued payload and returns whether publishing
  succeeded.
* `device.onCommand(command)` checks and consumes a matching inbound command.
* `device.isConnected()` reports the current MQTT connection state.
* `device.setBufferSize(bytes)` increases the maximum MQTT packet size when
  larger payloads are required.

Typed widgets are available for common values: `Level` and `Slider` use integers;
`Switch`, `Pushbutton`, and `Led` use booleans; `Photo` and `Map` use strings.
`Map` also accepts latitude and longitude directly:

```cpp
Map position("position", device);
position.write(41.902782, 12.496366);
```

## Examples

* **Basic** sends a minimal text payload.
* **PublishWidgets** publishes numbers, text, booleans, and typed widgets.
* **Commands** controls the Nano ESP32 built-in LED with `ledon` and `ledoff`.
* **MapWidget** publishes geographic coordinates.

For another board, keep using the generic `begin(..., Client&)` overload with
the network client supplied by that board's core. Configure the client’s TLS
certificate validation according to that board’s network library before calling
`begin(...)`.

Ethernet and GSM/LTE transports work the same way. Pass an SSL/TLS client such
as `EthernetSSLClient`, `GsmClientSecure`, or `TinyGsmClientSecure`:

```cpp
// The exact include and client type depend on the board and network library.
Client& netClient = yourTlsClient;
device.begin(SECRET_DEVICE_ID, SECRET_DEVICE_TOKEN, netClient);
```

The broker listens on a TLS-enabled MQTT port (`8243`), so plain
`EthernetClient` or `GsmClient` instances without TLS support cannot be used.

The library metadata intentionally declares `architectures=*`. This allows the
Library Manager to install it on any architecture because the library itself is
transport- and architecture-independent; compatibility depends on the selected
board providing a network client derived from Arduino `Client`.
