#pragma once
#ifndef __ATTENDANCE_H__
#define __ATTENDANCE_H__

#define ATTENDANCE_DEBUG

#include "Arduino.h"
#include "EspMQTTClient.h"

#include <mbedtls/base64.h>
#include <esp_heap_caps.h>

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/Concurrency/RTOS.h"

#define DAC_PIN_LRC 13
#define DAC_PIN_BCLK 12
#define DAC_PIN_DIN 14

// Global variables
// Audio
const size_t max_mp3_len = 64 * 1024; // 64KB
extern QueueStream<uint8_t> queue;
extern VolumeStream volume;
extern StreamCopy copier;

// MQTT
const size_t mqtt_chunk = 16 * 1024; // 16kb

// AudioTools functions
void audio_tools_setup();
void audio_tools_loop();
void restart_audio();
bool has_play_ended();

// MQTT functions
void mqtt_setup();
void onConnectionEstablished();
void mqtt_loop();
bool has_all_received();

// sinewave generator
// extern VolumeStream volumeStream;
void sinewave_generator_init();
void sinewave_generator_loop();

template <typename T>
inline T clamp(T val, T lo, T hi)
{
  return max(lo, min(hi, val));
}

#endif