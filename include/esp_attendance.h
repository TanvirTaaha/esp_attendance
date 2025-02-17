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

extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

// AsyncMQTT
#include <WiFi.h>
// WifiMan
#include <WiFiManager.h>
// Audio Playback
#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/Concurrency/RTOS.h>
// persistence
#include <EEPROM.h>
// crc & http downloader
#include "HTTPClient.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

// Global switch to enable/disable all debug logging
// Have to define before importing
#define ACTIVATE_LOGGING 1
#include "debug.h"

// I2S pins
#define DAC_PIN_LRC 27
#define DAC_PIN_BCLK 12
#define DAC_PIN_DIN 14

#define VOL_CTRL_INPUT
extern float vol;
extern char cmd;
#define AUDIO_COPIER_BUFFER_SIZE 2048
const int PING_TIME = 10 * 60 * 1000;  // units: milliseconds

// wifiman
void wifiman_setup();
void wifiman_loop();

// mqtt
extern bool is_subscribed;
void async_mqtt_setup();
void connectToMqtt();
void publish_ack(const char *msg);
const size_t max_mp3_buffer_size = 35 * 1024;
extern uint8_t *buffer_mp3;

// audio playback
extern volatile bool should_play;
extern URLStream url_stream;
extern StreamCopy copier;
extern I2SStream i2s;
extern VolumeStream volume;
extern EncodedAudioStream dec;
extern QueueHandle_t urlQueue;
void audio_init();
void restart_audio();
void start_url(const char *url);

// persistence
const int EEPROM_SIZE = 100;                  // unit: bytes
const int EEPROM_CREDS_ADDR = sizeof(float);  // After the sizeof(float) from starting
const int EEPROM_VOLUME_LEVEL_ADDR = 0;
void eeprom_setup();
void eeprom_wirte_str(char *buff, int len, int addr);
char *eeprom_read_str(int addr);
void eeprom_write_volume(float vol);
float eeprom_read_volume();
void eeprom_write_creds();
void eeprom_read_creds();

// crc & http downloader
bool downloadAndVerify(uint32_t expectedChecksum);
void setup_http();

#endif