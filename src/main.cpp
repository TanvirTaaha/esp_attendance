/**
 * @file main.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Main cpp file. contains arduino setup and loop functions
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "attendance.h"

Task copyTask("copy_task", 3000, 1, 0);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  audio_tools_setup();
  mqtt_setup();
  // sinewave_generator_init();

  copyTask.begin(
      []()
      {
        if (should_play)
        {
          static size_t copied_bytes;
          copied_bytes = copier.copy();
#ifdef ATTENDANCE_DEBUG
          Serial.printf("Copying stream: %d bytes\n", copied_bytes);
#endif
        }
        else
        {
#ifdef ATTENDANCE_DEBUG
          Serial.println("Not copying stream");
#endif
          vTaskDelay(pdMS_TO_TICKS(100));
        }
#ifdef ATTENDANCE_DEBUG
        Serial.printf("should_play:%d\n", should_play);
#endif
      });

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
    Serial.printf("queue.available():%d, \n", queue.available());
  }
#endif
  // sinewave_generator_loop();
  // restart_audio();
  mqtt_loop();
  // If already was playing(should_play) and then ended copying and has all received,
  // Restart audio
  if (queue.available() == 0 && has_all_received() && should_play)
  {
#ifdef ATTENDANCE_DEBUG
    Serial.println("Main loop. Play has ended. Restarting audio.");
#endif
    restart_audio();
  }
  // audio_tools_loop();
  // delay(1000);
}
