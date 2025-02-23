/**
 * @file audio_playback.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Implementations related to audio playback functions
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_attendance.h"

const int url_length = 100;

AudioInfo audio_info(44100, 1, 16);
URLStream url_stream;
I2SStream i2s;
VolumeStream volume(i2s);
EncodedAudioStream dec(&volume, new MP3DecoderHelix());
StreamCopy copier(dec, url_stream, AUDIO_COPIER_BUFFER_SIZE);

QueueHandle_t urlQueue;
Task play_task("play_task", 4 * AUDIO_COPIER_BUFFER_SIZE, 1, 0);  // core 0 (background core)

volatile bool should_play = false;

// for access from restart function
auto cfg = i2s.defaultConfig(TX_MODE);
auto vcfg = volume.defaultConfig();

// Need to be called after Serial.begin(Baud);
void audio_init() {
  LOG_DEBUG("in audio_init()");
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_ERROR) && ACTIVATE_LOGGING
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Error);
#endif
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_INFO) && ACTIVATE_LOGGING
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
#endif
  // begin processing
  cfg.pin_bck = DAC_PIN_BCLK;
  cfg.pin_ws = DAC_PIN_LRC;
  cfg.pin_data = DAC_PIN_DIN;
  cfg.copyFrom(audio_info);
  // cfg.sample_rate = memoryStream.audioInfo().sample_rate;
  // cfg.sample_rate = 44100;
  // cfg.channels = memoryStream.audioInfo().channels;
  i2s.begin(cfg);
  dec.begin(audio_info);

  // Voluime setup
  vcfg.copyFrom(cfg);
  vol = eeprom_read_volume();
  // if no volume is present
  if (vol <= 0.0f || vol >= 100.0f || isnanf(vol) || isinff(vol)) {
    LOG_WARN("Invalid volume on eeprom:%f", vol);
    vol = 1.0;
    eeprom_write_volume(vol);
  }
  vcfg.volume = vol;
  vcfg.allow_boost = true;
  volume.begin(vcfg);  // Have to be the last to begin()
  volume.setVolume(vol);

  // Start backgournd task
  play_task.begin([&]() {
    if (urlQueue == NULL) {
      urlQueue = xQueueCreate(10, url_length);
    }
    static char url[url_length];
    while (true) {
      if (xQueueReceive(urlQueue, &url, portMAX_DELAY) == pdTRUE) {
        start_url(url);
        if (should_play) {
          // yields are necessary to prevent watchdog reset as it is background task
          yield();
          copier.copyAll();
          yield();
          restart_audio();
          publish_ack("MAIN:audio successfully played completely");
          should_play = false;
        }
      }
    }
  });
  LOG_INFO("Audio started");
}

void start_url(const char *url) {
  url_stream.end();

  if (url_stream.begin(url, "audio/mp3")) {
    LOG_INFO("URL Stream started");
    should_play = true;
  } else {
    LOG_ERROR("Failed to start url stream");
    publish_ack("HTTP:Failed to start url stream");
    should_play = false;
  }
}

void restart_audio() {
  LOG_DEBUG("Restarting audio-tools");

  should_play = false;

  dec.flush();
  dec.end();  // Have to be called before it's downstreams ended
  LOG_DEBUG("After dec end");
  i2s.end();
  volume.end();
  copier.end();
  LOG_DEBUG("After copier end");

  delay(50);
  LOG_DEBUG("After delay 100");
  i2s.begin(cfg);
  LOG_DEBUG("After i2s begin");
  vcfg.volume = vol;
  vcfg.allow_boost = true;
  LOG_DEBUG("After volume begin:volume:%f", vol);
  dec.begin(audio_info);
  LOG_DEBUG("After dec begin");
  copier.begin(dec, url_stream);
  LOG_DEBUG("After copier begin");
  LOG_DEBUG("Audio RESTARTED");
  volume.begin(vcfg);  // Have to be the last to begin()
  volume.setVolume(vol);
}