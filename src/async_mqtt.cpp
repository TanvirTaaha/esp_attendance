#include "esp_attendance.h"
#include "mqtt_defines.h"

// Workaround to remedy linking issue since AsyncMQTT_ESP.h includes function implementations directly in the header file (AsyncMQTT_ESP32_Impl.h).
// This makes including the AsyncMQTT_ESP.h file in multiple cpp problematic. Have to sure that never happens.
#include <AsyncMQTT_ESP32.h>

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

// struct ChunkMetadata
// {
//     uint32_t chunk_number;
//     uint32_t total_chunks;
//     uint32_t chunk_size;
// } metadata;

void connectToWifi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event)
{
    switch (event)
    {
#if USING_CORE_ESP32_CORE_V200_PLUS

    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi ready");
        break;

    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi STA starting");
        break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("WiFi STA connected");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("WiFi lost IP");
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
#else

    case SYSTEM_EVENT_STA_GOT_IP:
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        connectToMqtt();
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        Serial.println("WiFi lost connection");
        xTimerStop(mqttReconnectTimer, 0); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        xTimerStart(wifiReconnectTimer, 0);
        break;
#endif

    default:
        break;
    }
}

void printSeparationLine()
{
    Serial.println("************************************************");
}

void onMqttConnect(bool sessionPresent)
{
    Serial.print("Connected to MQTT broker: ");
    Serial.print(MQTT_HOST);
    Serial.print(", port: ");
    Serial.println(MQTT_PORT);
    Serial.print("PubTopic: ");
    Serial.println(PubTopic);

    printSeparationLine();
    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    uint16_t packetIdSub = mqttClient.subscribe(PubTopic, 2);
    Serial.print("Subscribing at QoS 2, packetId: ");
    Serial.println(packetIdSub);

    // mqttClient.publish(PubTopic, 0, true, "ESP32 Test");
    // Serial.println("Publishing at QoS 0");

    // uint16_t packetIdPub1 = mqttClient.publish(PubTopic, 1, true, "test 2");
    // Serial.print("Publishing at QoS 1, packetId: ");
    // Serial.println(packetIdPub1);

    // uint16_t packetIdPub2 = mqttClient.publish(PubTopic, 2, true, "test 3");
    // Serial.print("Publishing at QoS 2, packetId: ");
    // Serial.println(packetIdPub2);

    printSeparationLine();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    (void)reason;

    Serial.println("Disconnected from MQTT.");

    if (WiFi.isConnected())
    {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void onMqttUnsubscribe(const uint16_t &packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total)
{
    // (void)payload;

    Serial.println("Publish received.");
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);
}

void onMqttPublish(const uint16_t &packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void async_mqtt_setup()
{
    Serial.print("\nStarting FullyFeature_ESP32 on ");
    Serial.println(ARDUINO_BOARD);
    Serial.println(ASYNC_MQTT_ESP32_VERSION);

    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0,
                                      reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0,
                                      reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    WiFi.onEvent(WiFiEvent);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();
}
