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

#include <string>

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

uint8_t *buffer_mp3 = nullptr;
bool is_subscribed = false;

char mqtt_topic[33];
char mqtt_topic_ack[37];
MqttPayloadStruct mqtt_payload_struct;
CredentialStruct credential_struct;

inline void print_creds() {
  LOG_DEBUG("Printing creds that is now loaded");
  LOG_DEBUG("device_id:\"%u\"", credential_struct.device_id);
  LOG_DEBUG("device_pass:\"%s\"", credential_struct.device_pass);
  LOG_DEBUG("mqtt_username:\"%s\"", credential_struct.mqtt_username);
  LOG_DEBUG("mqtt_pass:\"%s\"", credential_struct.mqtt_pass);
}

void connectToWifi() {
  LOG_INFO("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  eeprom_read_creds();
  print_creds();

  sprintf(mqtt_topic, "%s%d", MQTT_TOPIC_BASE, credential_struct.device_id);
  LOG_DEBUG("mqtt_topic:%s", mqtt_topic);
  sprintf(mqtt_topic_ack, "%s/ack", mqtt_topic);

  static char last_will_topic[38];
  sprintf(last_will_topic, "%s/will", mqtt_topic);
  static char last_will_msg[] = "Diconnected more than 60 seconds before\0";
  // sprintf(last_will_msg, "Diconnected more than 60 seconds before\0");

  static char client_id_str[10];
  sprintf(client_id_str, "ESP-%d", credential_struct.device_id);

  mqttClient.setWill(last_will_topic, 2, false, last_will_msg, strlen(last_will_msg));
  mqttClient.setKeepAlive(60);
  mqttClient.setCredentials(credential_struct.mqtt_username, credential_struct.mqtt_pass);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setClientId(client_id_str);

  LOG_INFO("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
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
      xTimerStop(mqttReconnectTimer, 0);  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
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
      xTimerStop(mqttReconnectTimer, 0);  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      xTimerStart(wifiReconnectTimer, 0);
      break;
#endif

    default:
      break;
  }
}

void printSeparationLine() {
  Serial.println("************************************************");
}

void onMqttConnect(bool sessionPresent) {
  LOG_INFO("Connected Successfully to MQTT broker: ");
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  Serial.print(MQTT_HOST);
  Serial.print(", port: ");
  Serial.println(MQTT_PORT);
#endif

  LOG_DEBUG("MqttTopic: %s", mqtt_topic);
#if CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_INFO && ACTIVATE_LOGGING
  printSeparationLine();
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
#endif

  uint16_t packetIdSub = mqttClient.subscribe(mqtt_topic, MQTT_QOS);
  LOG_DEBUG("Subscribing at QoS 2, packetId: %d", packetIdSub);

#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  printSeparationLine();
#endif
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  switch (reason) {
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS:
      LOG_ERROR("Disconnected from MQTT. MQTT_MALFORMED_CREDENTIALS");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED:
      LOG_ERROR("Disconnected from MQTT. MQTT_NOT_AUTHORIZED");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE:
      LOG_ERROR("Disconnected from MQTT. MQTT_SERVER_UNAVAILABLE");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED:
      LOG_ERROR("Disconnected from MQTT. MQTT_IDENTIFIER_REJECTED");
      break;
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
      LOG_ERROR("Disconnected from MQTT. MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
      break;
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED:
      LOG_ERROR("Disconnected from MQTT. TCP_DISCONNECTED");
      break;
    default:
      LOG_ERROR("Disconnected from MQTT. reason:%d", reason);
      break;
  }
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos) {
  is_subscribed = true;
  LOG_INFO("Subscribe acknowledged.");
  LOG_DEBUG("  packetId: %d\n  qos: %d", packetId, qos);
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  printSeparationLine();
#endif
}

void onMqttUnsubscribe(const uint16_t &packetId) {
  is_subscribed = false;
  LOG_INFO("Unsubscribe acknowledged.");
  LOG_DEBUG("  packetId: %d", packetId);
}

void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index, const size_t &total) {
  LOG_INFO("Message received.");
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  Serial.print("  topic: ");
  Serial.println(topic);
  // Serial.print("  qos: ");
  // Serial.println(properties.qos);
  // Serial.print("  dup: ");
  // Serial.println(properties.dup);
  // Serial.print("  retain: ");
  // Serial.println(properties.retain);
  // Serial.print("  len: ");
  // Serial.println(len);
  // Serial.print("  index: ");
  // Serial.println(index);
  // Serial.print("  total: ");
  // Serial.println(total);
#endif

  LOG_DEBUG("Received message:%s", payload);
  payload[len - 1] = '\0';  // Ensure null-termination

  if (strstr(topic, mqtt_topic)) {
    yield();
    if (len == total) {
      int ret = sscanf(payload, "%u_%llu_%u_%u_%u\n\0", &mqtt_payload_struct.msg_id, &mqtt_payload_struct.timestamp, &mqtt_payload_struct.stuff_id, &mqtt_payload_struct.file_size, &mqtt_payload_struct.checksum);
      LOG_INFO("msg_id:%u, timestamp:%llu, stuff_id:%u, file_size:%u, checksum:%u", mqtt_payload_struct.msg_id, mqtt_payload_struct.timestamp, mqtt_payload_struct.stuff_id, mqtt_payload_struct.file_size, mqtt_payload_struct.checksum);
      if (ret == 5) {
        LOG_DEBUG("parsing SUCCESS");
        yield();
        while (should_play) {
          delay(50);
        }
        if (downloadAndVerify(mqtt_payload_struct.checksum)) {
          audio_data.setValue((uint8_t *)buffer_mp3, mqtt_payload_struct.file_size);
          audio_data.resize(mqtt_payload_struct.file_size);
          restart_audio();
          should_play = true;
        }
      } else {
        LOG_ERROR("mqtt message parsing FAILED");
        publish_ack("MQTT:Parsing failed from mqtt payload");
      }
    }
  }
}

void onMqttPublish(const uint16_t &packetId) {
  LOG_INFO("Publish acknowledged.");
  // LOG_DEBUG("  packetId: %d", packetId);
}

void async_mqtt_setup() {
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

  wifiman_setup();

  buffer_mp3 = (uint8_t *)malloc(max_mp3_buffer_size + 1);  // 1 extra to avoid overflow
}

void publish_ack(const char *msg) {
  uint16_t packetIdPub2 = mqttClient.publish(mqtt_topic_ack, MQTT_QOS, false, msg, strlen(msg));
  // LOG_INFO("Publishing ack at QoS 2, packetId: %d, msg:%s", packetIdPub2, msg);
}
