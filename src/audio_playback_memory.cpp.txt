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
AudioInfo audio_info(44100, 1, 16);
MemoryStream audio_data = MemoryStream(buffer_mp3, buffer_size_orig);
I2SStream i2s;
VolumeStream volume(i2s);
EncodedAudioStream dec(&volume, new MP3DecoderHelix());
StreamCopy copier(dec, audio_data, AUDIO_COPIER_BUFFER_SIZE);

volatile bool should_play = false;

// for access from restart function
auto cfg = i2s.defaultConfig(TX_MODE);
auto vcfg = volume.defaultConfig();

// Need to be called after Serial.begin(Baud);
void audio_init()
{
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
  if (vol <= 0.0f || vol >= 100.0f || isnanf(vol) || isinff(vol))
  {
    LOG_WARN("Invalid volume on eeprom:%f", vol);
    vol = 1.0;
    eeprom_write_volume(vol);
  }
  vcfg.volume = vol;
  vcfg.allow_boost = true;
  volume.begin(vcfg); // Have to be the last to begin()
  volume.setVolume(vol);

  LOG_INFO("Audio started");
}

void restart_audio()
{
  LOG_DEBUG("Restarting audio-tools");

  should_play = false;

  dec.flush();
  dec.end(); // Have to be called before it's downstreams ended
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
  copier.begin(dec, audio_data);
  LOG_DEBUG("After copier begin");
  LOG_DEBUG("Audio RESTARTED");
  volume.begin(vcfg); // Have to be the last to begin()
  volume.setVolume(vol);
}