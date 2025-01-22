/**
 * @file esp_attendance.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Main cpp file containing arduino setup and loop functions
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_attendance.h"

void setup()
{
  Serial.begin(115200);
  while (!Serial && millis() < 2000)
    Serial.print(".");
  Serial.println();
  LOG_DEBUG("Serial Started");

  LOG_INFO("\nStarting Attendance Greeter on ESP32-%s", ARDUINO_BOARD);

  audio_init();
  async_mqtt_setup();
  LOG_DEBUG("MQTT started.");
}

void loop()
{
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  static unsigned long last_millis = millis();
  if ((millis() - last_millis) > 5000)
  {
    Serial.printf("main loop. subscribed:%s\n", is_subscribed ? "true" : "false");
    last_millis = millis();
  }
#endif
}
