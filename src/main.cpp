#include "attendance.h"

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  // audio_tools_setup();
  mqtt_setup();
  // sinewave_generator_init();
#ifdef ATTENDANCE_DEBUG
  Serial.println("Started:setup done.");
#endif
}

void loop()
{
  // if (Serial.available() > 0)
  // {
  //   static char cmd;
  //   cmd = Serial.read();
  //   if (cmd == 'V')
  //   {
  //     volumeStream.setVolume(clamp(volumeStream.volume(0) + 0.02f, 0.0f, 1.0f));
  //     // Serial.printf("volume up:%f\n", volumeStream.volume(0));
  //   }
  //   else if (cmd == 'v')
  //   {
  //     volumeStream.setVolume(clamp(volumeStream.volume(0) - 0.02f, 0.0f, 1.0f));
  //     // Serial.printf("volume down:%f\n", volumeStream.volume(0));
  //   }
  // }
#ifdef ATTENDANCE_DEBUG
  static long lasttime = millis();
  if (millis() - lasttime >= 1000)
  {
    lasttime = millis();
    Serial.println("main loop...");
  }
#endif
  // sinewave_generator_loop();
  // restart_audio();
  mqtt_loop();
  // audio_tools_loop();
  // delay(1000);
}
