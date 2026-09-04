#include "MicroedenConnect.h"

#if defined(ARDUINO_ARCH_ESP32)
#include <WiFiClientSecure.h>
#include "MicroedenRootCA.h"
#endif

MicroedenConnect* MicroedenConnect::_instance = nullptr;

void MicroedenConnect::mqttCallback(char* topic, byte* payload, unsigned int length) {
    // PubSubClient provides the topic for every message; this library uses one
    // subscribed inbox topic, so only the JSON payload needs to be retained.
    (void)topic;
    if (_instance) {
        deserializeJson(_instance->_lastInboundDoc, payload, length);
    }
}

MicroedenConnect::MicroedenConnect() : _mqtt() {
    _instance = this;
}

void MicroedenConnect::begin(const char* deviceId, const char* token, Client& netClient) {
    _deviceId = String(deviceId);
    _token = token;
    _client = &netClient;

    memset(_host, 0, sizeof(_host));
    snprintf(_host, sizeof(_host), "%s", "microeden.io");
    
    // The device ID identifies the MQTT credentials and topics; the broker is
    // shared by all devices and is addressed through the fixed public host.
    _clientId = "MICROEDEN-MYDEVICE-" + _deviceId;
    _topicIn = "microeden/dot/" + _deviceId + "/inbox";
    _topicOut = "microeden/dot/" + _deviceId + "/outbox";
    
    _mqtt.setSocketTimeout(15);
    _mqtt.setClient(*_client);
    _mqtt.setServer(_host, _port); 
    _mqtt.setCallback(mqttCallback);
}

#if defined(ARDUINO_ARCH_ESP32)
void MicroedenConnect::begin(const char* deviceId, const char* token, WiFiClientSecure& netClient) {
    netClient.setCACert(MICROEDEN_ROOT_CA);
    begin(deviceId, token, static_cast<Client&>(netClient));
}
#endif

void MicroedenConnect::writeKeyWord(const char* key, const char* value) { _payloadDoc[key] = value; }
void MicroedenConnect::writeKeyWord(const char* key, double value) { _payloadDoc[key] = value; }
void MicroedenConnect::writeKeyWord(const char* key, float value) { _payloadDoc[key] = value; }
void MicroedenConnect::writeKeyWord(const char* key, int value) { _payloadDoc[key] = value; }
void MicroedenConnect::writeKeyWord(const char* key, bool value) { _payloadDoc[key] = value; }
void MicroedenConnect::writeKeyWord(const char* key, const String& value) { _payloadDoc[key] = value; }

bool MicroedenConnect::send() {
    if (_mqtt.connected()) {
        String output;
        serializeJson(_payloadDoc, output);
        bool success = _mqtt.publish(_topicOut.c_str(), output.c_str());
        _payloadDoc.clear(); 
        return success;
    }
    return false;
}

bool MicroedenConnect::isConnected() {
    return _mqtt.connected();
}

void MicroedenConnect::run() {
    if (!_mqtt.connected()) {
        // Reconnect transparently so applications only need to call run().
        if (_mqtt.connect(_clientId.c_str(), _deviceId.c_str(), _token.c_str())) {
            _mqtt.subscribe(_topicIn.c_str());   
        }
    }
    _mqtt.loop();
}

bool MicroedenConnect::onCommand(const char* expectedCmd, const char* key) {
    if (_lastInboundDoc.containsKey(key)) {
        String currentCmd = _lastInboundDoc[key].as<String>();
        if (currentCmd == expectedCmd) {
            _lastInboundDoc.remove(key);
            return true;
        }
    }
    return false;
}

void MicroedenConnect::setBufferSize(uint16_t size) {
    _mqtt.setBufferSize(size); 
}
