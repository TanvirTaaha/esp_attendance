/**
 * @file esp_attendance.h
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Contains all the includes.
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#ifndef __ESP_ATTENDANCE_H__
#define __ESP_ATTENDANCE_H__

#include <Arduino.h>
#include <mbedtls/base64.h>

// AsyncMQTT
#include <WiFi.h>

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

// Audio Playback
#include <AudioTools.h>
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <AudioTools/Concurrency/RTOS.h>

// Global switch to enable/disable all debug logging
// Have to define before importing
#define ACTIVATE_LOGGING 1
#include "debug.h"

// mqtt
extern bool is_subscribed;
void async_mqtt_setup();

// audio playback
const int buffer_size_base64 = 20*1024;
const int buffer_size_orig = (buffer_size_base64 * 3) / 4; // for no padding
extern BufferRTOS<uint8_t> bufferRTOS;
extern QueueStream<uint8_t> queue;
void audio_init();
#endif