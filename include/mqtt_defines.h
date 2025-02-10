/**
 * @file mqtt_defines.h
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Parameters for wifi and mqtt configuration
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#ifndef __MQTT_DEFINES_H__
#define __MQTT_DEFINES_H__

#define _ASYNC_MQTT_LOGLEVEL_ 1

#define WIFI_SSID "MISINFRA"
#define WIFI_PASSWORD "123456#@227"

#define MQTT_HOST IPAddress(192, 168, 21, 44)  // taaha
// #define MQTT_HOST IPAddress(192, 168, 142, 126) // taaha on Taaha's Pixel
// #define MQTT_HOST IPAddress(192, 168, 21, 35)  // gpuserver2x
// #define MQTT_HOST IPAddress(192, 168, 142, 111)  // gpuserver2x on Taaha's Pixel
// #define MQTT_HOST "broker.emqx.io" // Broker address
#define MQTT_PORT 1883

#define MQTT_QOS 2

#define MQTT_TOPIC_BASE "FaceRecognition/potpot/"
extern char mqtt_topic[33];      // topic structure "FaceRecognition/potpot/" + "<uint32 in decimal>"
extern char mqtt_topic_ack[37];  // topic structure "FaceRecognition/potpot/" + "<uint32 in decimal>" + "/ack"

struct MqttPayloadStruct {
  uint32_t msg_id;
  uint32_t stuff_id;
  uint32_t file_size;
  uint32_t checksum;
  uint64_t timestamp;
};
extern MqttPayloadStruct mqtt_payload_struct;
struct CredentialStruct {
  uint32_t device_id;
  char device_pass[16];
  char mqtt_pass[16];
  char mqtt_username[20];
};
extern CredentialStruct credential_struct;
#endif