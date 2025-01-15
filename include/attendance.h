#pragma once
#ifndef __ATTENDANCE_H__
#define __ATTENDANCE_H__

#define ATTENDANCE_DEBUG

#include "Arduino.h"
#include "EspMQTTClient.h"
#include <esp_heap_caps.h>

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

#define DAC_PIN_LRC 13
#define DAC_PIN_BCLK 12
#define DAC_PIN_DIN 14

// Global variables
const size_t max_mp3_len = 10 * 1024;
extern uint8_t mp3Buffer[max_mp3_len];

// AudioTools functions
void audio_tools_setup();
void audio_tools_loop();
void restart_audio();

// MQTT functions
void mqtt_setup();
void onConnectionEstablished();
void mqtt_loop();

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