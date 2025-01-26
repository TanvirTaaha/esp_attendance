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

// #define MQTT_HOST IPAddress(192, 168, 21, 44) // taaha
#define MQTT_HOST IPAddress(192, 168, 21, 35) // gpuserver2x
// #define MQTT_HOST "broker.emqx.io" // Broker address
#define MQTT_PORT 1883

#define MQTT_QOS 2

const char *PubTopic = "streams/audio"; // Topic to publish

// chunk sizes

#endif