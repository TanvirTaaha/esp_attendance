/**
 * @file espmqtt_client.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Implementaion of fucntions related to networking, mqtt and file chunking
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "attendance.h"

EspMQTTClient mqttClient(
    "MISINFRA",
    "123456#@227",
    "192.168.21.44", // MQTT Broker server ip
    "ESP",           // Client name that uniquely identify your device
    1883             // The MQTT port, default to 1883. this line can be omitted
);

struct ChunkMetadata
{
  uint32_t chunk_number;
  uint32_t total_chunks;
  uint32_t chunk_size;
} metadata;

void mqtt_setup()
{
#ifdef ATTENDANCE_DEBUG
  Serial.println("mqtt_setup called");
#endif
  // Optional functionalities of EspMQTTClient
  mqttClient.setMaxPacketSize(max_mqtt_chunk + ((sizeof(ChunkMetadata) * 4) / 3)); // Have to take the length after base64 encoding
  mqttClient.enableDebuggingMessages();                                            // Enable debugging messages sent to serial output
  mqttClient.enableHTTPWebUpdater();                                               // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  mqttClient.enableOTA();                                                          // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  mqttClient.enableLastWillMessage("ESP/lastwill", "I am going offline");          // You can activate the retain flag by setting the third parameter to true
  mqttClient.enableMQTTPersistence();
#ifdef ATTENDANCE_DEBUG
  Serial.println("MQTT initialized");
#endif
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
#ifdef ATTENDANCE_DEBUG
  Serial.println("[onConnectionEstablished]Connected to MQTT broker");
#endif

  // Subscribe to "mytopic/test" and display received message to Serial
  mqttClient.subscribe(
      "FaceRecognition/match_id",
      [](const String &payload)
      {
        Serial.printf("Message on FaceRecognition/match_id:%s\n", payload.c_str());
      },
      mqtt_qos);

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  mqttClient.subscribe(
      "streams/#",
      [](const String &topic, const String &payload)
      {
        Serial.printf("From wildcard:%s\n", topic.c_str());
        Serial.printf("Payload size:%u bytes\n", payload.length());
        Serial.printf("base64 that was received:\"%s\"\n", payload.c_str());
        if (payload.length() >= sizeof(ChunkMetadata))
        {
          size_t expected_decoded_len = ((4 * payload.length() / 4) + 3) & ~3;
          Serial.printf("expected length:%lu\n", expected_decoded_len);
          uint8_t *decoded_string = (uint8_t *)malloc(expected_decoded_len);
          size_t decoded_bytes_written;
          auto ret = mbedtls_base64_decode(decoded_string, expected_decoded_len, &decoded_bytes_written, (uint8_t *)payload.c_str(), payload.length());
          if (ret == 0)
          {
            Serial.printf("decoded successfully: bytes_written:%lu\n", decoded_bytes_written);
            // Print every bytes in decoded data
            // for (size_t i = 0; i < decoded_bytes_written; i++)
            // {
            //   Serial.printf("%d: %d, ", i, decoded_string[i]);
            // }
            // Serial.println();
            memcpy(&metadata, decoded_string, sizeof(ChunkMetadata));
            Serial.printf("chunk number:%u, total_chunks:%u, chunk_size:%u\n", metadata.chunk_number, metadata.total_chunks, metadata.chunk_size);

            // if (metadata.chunk_size != (decoded_bytes_written - sizeof(ChunkMetadata)))
            // {
            //   Serial.println("\nPayload rest of the spaces are not equal to chunk size\n");
            //   Serial.printf("\n\nchunk size:%u, (payload.length() - sizeof(ChunkMetadata)):%lu\n\n", metadata.chunk_size, (decoded_bytes_written - sizeof(ChunkMetadata)));
            // }

            size_t written = queue.write((uint8_t *)decoded_string + sizeof(ChunkMetadata), decoded_bytes_written - sizeof(ChunkMetadata));
            if (written != metadata.chunk_size)
            {
              Serial.printf("Queue full! Only wrote %d/%d bytes\n", written, metadata.chunk_size);
              // should_play = false;
            }
            // should_play = queue.available() > (metadata.chunk_size * 2);
            should_play = (metadata.chunk_number + 1) == metadata.total_chunks;
            Serial.printf("Setting should play to true:%d\n", should_play);
          }
          else
          {
            Serial.printf("Base64 conversion failed. error-code:%d\n", ret);
          }
          free(decoded_string);
        }
        else
        {
          Serial.printf("payload size small: expected:%lu, got:%lu\n", sizeof(ChunkMetadata), payload.length());
          Serial.print("Payload:");
          Serial.println(payload);
        }
      },
      mqtt_qos);

  // Publish a message to "mytopic/test"
  // mqttClient.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  // mqttClient.executeDelayed(5 * 1000, []()
  //                       { mqttClient.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later"); });
}

void mqtt_loop()
{
  mqttClient.loop();
}

bool has_all_received()
{
  return (metadata.chunk_number + 1) == metadata.total_chunks;
}