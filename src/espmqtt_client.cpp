/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
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
  uint8_t chunk_number;
  uint8_t total_chunks;
  uint8_t chunk_size;
} metadata;

void mqtt_setup()
{
#ifdef ATTENDANCE_DEBUG
  Serial.println("mqtt_setup called");
#endif
  // Optional functionalities of EspMQTTClient
  mqttClient.setMaxPacketSize(mqtt_chunk + sizeof(ChunkMetadata));
  mqttClient.enableDebuggingMessages();                                   // Enable debugging messages sent to serial output
  mqttClient.enableHTTPWebUpdater();                                      // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  mqttClient.enableOTA();                                                 // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  mqttClient.enableLastWillMessage("ESP/lastwill", "I am going offline"); // You can activate the retain flag by setting the third parameter to true
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
      });

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  mqttClient.subscribe(
      "streams/#",
      [](const String &topic, const String &payload)
      {
        Serial.println("(From wildcard) topic: " + topic);
        Serial.println();
        Serial.print("PayPayload size:");
        Serial.print(payload.length());
        Serial.println(" bytes");
        if (payload.length() >= sizeof(ChunkMetadata))
        {
          memcpy(&metadata, payload.c_str(), sizeof(ChunkMetadata));
          Serial.printf("chunk number:%d, total_chunks:%d, chunk_size:%d\n", metadata.chunk_number, metadata.chunk_size, metadata.chunk_size);
          if (payload.length() != (metadata.chunk_size + sizeof(ChunkMetadata)))
            Serial.println("Packet size does not match");
        }
        else
        {
          Serial.printf("payload size small: expected:%lu, got:%lu\n", sizeof(ChunkMetadata), payload.length());
          Serial.print("Payload:");
          Serial.println(payload);
        }
        Serial.println("Printing every bytes:");
        for (int i = 0; i < payload.length(); i++)
        {
          Serial.printf("0x%x ", payload.c_str()[i]);
          if (i % 9 == 0)
          {
            Serial.println();
          }
        }
        Serial.println();

        size_t written = queue.write((uint8_t *)payload.c_str() + sizeof(ChunkMetadata), payload.length() - sizeof(ChunkMetadata));
        if (written != metadata.chunk_size)
        {
          Serial.printf("Queue full! Only wrote %d/%d bytes\n", written, metadata.chunk_size);
        }
      });

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
  return metadata.chunk_number == metadata.total_chunks;
}