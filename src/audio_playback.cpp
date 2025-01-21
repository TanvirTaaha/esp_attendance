#include "attendance.h"

BufferRTOS<uint8_t> bufferRTOS(max_mp3_len);
QueueStream<uint8_t> queue(bufferRTOS);
I2SStream i2s;
VolumeStream volume(i2s);
EncodedAudioStream decoder(&volume, new MP3DecoderHelix); // output to decoder
StreamCopy copier(decoder, queue);                        // copy into out that decodes and outputs to i2s
volatile bool should_play = false;

// for access from restart function
auto cfg = i2s.defaultConfig(TX_MODE);
auto vcfg = volume.defaultConfig();

/*
  Need to be called after Serial.being(Baud)
*/
void audio_tools_setup()
{
#ifdef ATTENDANCE_DEBUG
  // AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
#else
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Error);
#endif

  // begin processing
  cfg.pin_bck = DAC_PIN_BCLK;
  cfg.pin_ws = DAC_PIN_LRC;
  cfg.pin_data = DAC_PIN_DIN;
  // cfg.sample_rate = memoryStream.audioInfo().sample_rate;
  // cfg.sample_rate = 44100;
  // cfg.channels = memoryStream.audioInfo().channels;
  i2s.begin(cfg);

  // Voluime setup
  vcfg.copyFrom(cfg);
  volume.begin(vcfg);
  volume.setVolume(0.2);

  decoder.begin();
  queue.begin();

#ifdef ATTENDANCE_DEBUG
  Serial.println("Audio began");
#endif
}

void restart_audio()
{
#ifdef ATTENDANCE_DEBUG
  Serial.println("restarting audio-tools");
#endif
  should_play = false;

  i2s.end();
  volume.end();
  decoder.end();
  queue.end();

  i2s.begin(cfg);
  volume.begin(vcfg);
  decoder.begin();
  queue.begin();

#ifdef ATTENDANCE_DEBUG
  Serial.println("Audio RESTARTED");
#endif
}
