/**
 * @file async_mqtt.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Implementations of mqtt and network related fucntions
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_attendance.h"
#include "mqtt_defines.h"

// Workaround to remedy linking issue since AsyncMQTT_ESP.h includes function implementations directly in the header file (AsyncMQTT_ESP32_Impl.h).
// This makes including the AsyncMQTT_ESP.h file in multiple cpp problematic. Have to sure that never happens.
#include <AsyncMQTT_ESP32.h>

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

bool is_subscribed = false;

void connectToWifi()
{
  LOG_INFO("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt()
{
  LOG_INFO("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
#if USING_CORE_ESP32_CORE_V200_PLUS

  case ARDUINO_EVENT_WIFI_READY:
    LOG_INFO("WiFi ready");
    break;

  case ARDUINO_EVENT_WIFI_STA_START:
    LOG_INFO("WiFi STA starting");
    break;

  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
    LOG_INFO("WiFi STA connected");
    break;

  case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    LOG_INFO("WiFi connected.");
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif
    connectToMqtt();
    break;

  case ARDUINO_EVENT_WIFI_STA_LOST_IP:
    LOG_ERROR("WiFi lost IP");
    break;

  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    LOG_ERROR("WiFi lost connection");
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
  LOG_INFO("Connected Successfully to MQTT broker: ");
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  Serial.print(MQTT_HOST);
  Serial.print(", port: ");
  Serial.println(MQTT_PORT);
#endif

  LOG_INFO("PubTopic: %s", PubTopic);
#if CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_INFO
  printSeparationLine();
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
#endif

  uint16_t packetIdSub = mqttClient.subscribe(PubTopic, MQTT_QOS);
  LOG_DEBUG("Subscribing at QoS 2, packetId: %d", packetIdSub);

#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  printSeparationLine();
#endif
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void)reason;

  LOG_ERROR("Disconnected from MQTT.");

  if (WiFi.isConnected())
  {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
  is_subscribed = true;
  LOG_INFO("Subscribe acknowledged.");
  LOG_DEBUG("  packetId: %d\n  qos: %d", packetId, qos);
}

void onMqttUnsubscribe(const uint16_t &packetId)
{
  is_subscribed = false;
  LOG_INFO("Unsubscribe acknowledged.");
  LOG_DEBUG("  packetId: %d", packetId);
}

void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total)
{
  LOG_INFO("Message received.");
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
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
#endif
  payload[len] = '\0';
  LOG_DEBUG("Received message:%s", payload);

  if (index == 0)
  {
    LOG_DEBUG("Emptying queue since new data stream detected");
  }
  // Copy if only it will fit have to be '<'
  if (index + len < buffer_size_base64)
  {
    size_t written = queue.write((uint8_t *)payload, len);
    LOG_DEBUG("written:%d, len:%d", written, len);
  }
  else
  {
    LOG_ERROR("Buffer overflow last packet(s) are being discarded");
  }
  // print after the last packet is received(even not copied)
  if (index + len == total)
  {
    Serial.println("Full Message:");
    char buffer[100];
    while (queue.available())
    {
      size_t bytes_read = queue.readBytes((uint8_t *)buffer, MIN(100, queue.available()));
      buffer[bytes_read] = '\0';
      Serial.print(buffer);
    }
    Serial.println();
  }
}

void onMqttPublish(const uint16_t &packetId)
{
  LOG_INFO("Publish acknowledged.");
  LOG_DEBUG("  packetId: %d", packetId);
}

void async_mqtt_setup()
{
  LOG_DEBUG("%s", ASYNC_MQTT_ESP32_VERSION);
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
