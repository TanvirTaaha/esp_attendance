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

// WifiMan
#include <WiFiManager.h>

// Audio Playback
#include <AudioTools.h>
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include <AudioTools/Concurrency/RTOS.h>

// Global switch to enable/disable all debug logging
// Have to define before importing
#define ACTIVATE_LOGGING 1
#include "debug.h"

// I2S pins
#define DAC_PIN_LRC 13
#define DAC_PIN_BCLK 12
#define DAC_PIN_DIN 14

#define VOL_CTRL_INPUT
extern float vol;
extern char cmd;
#define DELAY_AFTER_COPY_ENDED 2000
#define AUDIO_COPIER_BUFFER_SIZE 2048

// wifiman
void wifiman_setup();
void wifiman_loop();

// mqtt
extern bool is_subscribed;
void async_mqtt_setup();
void connectToMqtt();
const size_t buffer_size_base64 = 35 * 1024;
const size_t buffer_size_orig = ((buffer_size_base64 + 3) / 4) * 3; // for no padding
extern char *buffer_base64;
extern uint8_t *buffer_mp3;

// audio playback
extern volatile bool should_play;
extern MemoryStream audio_data;
extern StreamCopy copier;
extern I2SStream i2s;
extern VolumeStream volume;
extern EncodedAudioStream dec;
void audio_init();
void restart_audio();
#endif