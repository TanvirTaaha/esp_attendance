#include "attendance.h"

uint8_t mp3Buffer[max_mp3_len];
MemoryStream memoryStream(mp3Buffer, max_mp3_len);
I2SStream i2s;
MP3DecoderHelix decoder;
EncodedAudioStream out(&i2s, &decoder); // output to decoder
StreamCopy copier(out, memoryStream);   // copy into out that decodes and outputs to i2s

auto cfg = i2s.defaultConfig(TX_MODE);

/*
  Need to be called after Serial.being(Baud)
*/
void audio_tools_setup()
{
#ifdef ATTENDANCE_DEBUG
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
#else
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Error);
#endif

  // begin processing
  cfg.pin_bck = DAC_PIN_BCLK;
  cfg.pin_ws = DAC_PIN_LRC;
  cfg.pin_data = DAC_PIN_DIN;
  cfg.sample_rate = memoryStream.audioInfo().sample_rate;
  // cfg.sample_rate = 44100;
  cfg.channels = memoryStream.audioInfo().channels;
  i2s.begin(cfg);
  out.begin();
#ifdef ATTENDANCE_DEBUG
  Serial.println("Audio began");
#endif
}

void restart_audio()
{
  decoder.end();
  out.end();
  i2s.end();
  memoryStream.end();

  memoryStream.begin();
  decoder.begin();
  i2s.begin(cfg);
  out.begin();
#ifdef ATTENDANCE_DEBUG
  Serial.println("Audio RESTARTED");
#endif
}

void audio_tools_loop()
{
  if (memoryStream.available())
  {
    copier.copy();
  }
  else
  {
    decoder.end(); // flush output
    auto info = out.decoder().audioInfo();
    LOGI("The audio rate from the mp3 file is %d", info.sample_rate);
    LOGI("The channels from the mp3 file is %d", info.channels);
    i2s.end();
#ifdef ATTENDANCE_DEBUG
    Serial.println("AudioTools Stop called.");
#endif
    stop();
  }
}