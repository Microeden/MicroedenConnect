#ifndef MICROEDEN_CONNECT_H
#define MICROEDEN_CONNECT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFiClientSecure.h>
#endif

/**
 * @brief MQTT client used to connect an Arduino-compatible device to MicroEden.
 *
 * The class queues key/value pairs in a JSON payload, publishes them to the
 * device outbox topic, and dispatches commands received on the inbox topic.
 * Call run() frequently from loop() to keep the MQTT connection alive.
 */
class MicroedenConnect {
private:
    char _host[64];
    const uint16_t _port = 8243;
    String _deviceId;
    String _token;
    String _clientId;
    String _topicIn;
    String _topicOut;
    
    Client* _client = nullptr;
    PubSubClient _mqtt;
    JsonDocument _payloadDoc;     
    JsonDocument _lastInboundDoc; 

    static MicroedenConnect* _instance;
    static void mqttCallback(char* topic, byte* payload, unsigned int length);

public:
    /** Creates an unconfigured MicroEden client. */
    MicroedenConnect();

    /**
     * @brief Configures the client with a generic Arduino Client.
     * @param deviceId MicroEden device identifier.
     * @param token MicroEden device access token.
     * @param netClient Network client implementation used for MQTT traffic.
     */
    void begin(const char* deviceId, const char* token, Client& netClient);

#if defined(ARDUINO_ARCH_ESP32)
    /**
     * @brief Configures an ESP32 TLS client and the bundled MicroEden CA.
     * @param deviceId MicroEden device identifier.
     * @param token MicroEden device access token.
     * @param netClient ESP32 secure network client.
     */
    void begin(const char* deviceId, const char* token, WiFiClientSecure& netClient);
#endif

    /** Adds a string field to the next JSON payload. */
    void writeKeyWord(const char* key, const char* value);
    /** Adds a double-precision field to the next JSON payload. */
    void writeKeyWord(const char* key, double value); 
    /** Adds a single-precision field to the next JSON payload. */
    void writeKeyWord(const char* key, float value);
    /** Adds an integer field to the next JSON payload. */
    void writeKeyWord(const char* key, int value);
    /** Adds a boolean field to the next JSON payload. */
    void writeKeyWord(const char* key, bool value);
    /** Adds an Arduino String field to the next JSON payload. */
    void writeKeyWord(const char* key, const String& value);

    /**
     * @brief Checks whether an inbound command matches the expected value.
     * @param expectedCmd Command value to match.
     * @param key JSON field containing the command.
     * @return true when a matching command was found and consumed.
     */
    bool onCommand(const char* expectedCmd, const char* key = "content");

    /** Reads a value from the last inbound JSON document. */
    template <typename T>
    T readKeyWord(const char* key = "content") {
        return _lastInboundDoc[key].as<T>();
    }

    /** Maintains the connection, reconnects when necessary, and processes MQTT traffic. */
    void run();
    /** Publishes the queued JSON payload and clears it after the publish attempt. */
    bool send();
    /** @return true when the MQTT session is currently connected. */
    bool isConnected();

    /** Sets the maximum MQTT packet size accepted by PubSubClient. */
    void setBufferSize(uint16_t size);
};

/**
 * @brief Base class for typed MicroEden widgets.
 * @tparam T C++ type used to read and write the widget value.
 */
template <typename T>
class CloudWidget {
protected:
    const char* _key;
    MicroedenConnect* _device;
public:
    /** Creates a widget bound to a JSON key and a MicroEden client. */
    CloudWidget(const char* key, MicroedenConnect& device) : _key(key), _device(&device) {}

    /** Queues a new value for this widget in the next payload. */
    void write(T value) { 
        _device->writeKeyWord(_key, value); 
    }

    /** Reads this widget's value from the last inbound JSON document. */
    T read() { 
        return _device->readKeyWord<T>(_key); 
    }
};

/** Integer-valued level widget. */
class Level      : public CloudWidget<int>    { public: Level(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} };
/** Integer-valued slider widget. */
class Slider     : public CloudWidget<int>    { public: Slider(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} };
/** Boolean switch widget. */
class Switch     : public CloudWidget<bool>   { public: Switch(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} };
/** Boolean pushbutton widget. */
class Pushbutton : public CloudWidget<bool>   { public: Pushbutton(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} };
/** Boolean LED widget. */
class Led        : public CloudWidget<bool>   { public: Led(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} };
/** String-valued photo widget. */
class Photo      : public CloudWidget<String> { public: Photo(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} };
/** String-valued map widget, with a latitude/longitude convenience overload. */
class Map        : public CloudWidget<String> { 
public: 
    Map(const char* k, MicroedenConnect& d) : CloudWidget(k, d) {} 

    /** Queues coordinates formatted as "latitude,longitude". */
    void write(double lat, double lng) {
        String coord = String(lat, 6) + "," + String(lng, 6);
        _device->writeKeyWord(_key, coord);
    }

    /** Queues a preformatted map value. */
    void write(const String& value) {
        CloudWidget<String>::write(value);
    }
};

#endif  // MICROEDEN_CONNECT_H
