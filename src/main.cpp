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

float vol = 0.0;
char cmd;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000)
    Serial.print(".");
  Serial.println();
  LOG_DEBUG("Serial Started");

  LOG_INFO("\nStarting Attendance Greeter on ESP32-%s", ARDUINO_BOARD);

  eeprom_setup();
  audio_init();

  async_mqtt_setup();
  setup_http();
}

void loop() {
#if (CURRENT_LOG_LEVEL >= ATTENDACE_LOG_LEVEL_DEBUG) && ACTIVATE_LOGGING
  static unsigned long last_millis = millis();
  if ((millis() - last_millis) > 5000) {
    Serial.printf("main loop. subscribed:%s\n", is_subscribed ? "true" : "false");
    last_millis = millis();
  }
#endif

  if (Serial.available() > 0) {
    cmd = Serial.read();
    switch (cmd) {
      case 'V':
        vol += 0.02;
        volume.setVolume(vol);
        // LOG_INFO("Volume level increased to: %f", vol);
        break;
      case 'v':
        vol -= 0.02;
        volume.setVolume(vol);
        // LOG_INFO("Volume level decreased to: %f", vol);
        break;
      default:
        // LOG_INFO("Invalid Command, current volume level:%f", vol);
        break;
    }
    eeprom_write_volume(vol);
  }

  if (should_play) {
    yield();
    copier.copyAll();
    dec.writeSilence(1000);
    yield();
    copier.copyAll();
    // restart_audio(); // will be called before playing next audio
    publish_ack("MAIN:audio successfully played completely");
    should_play = false;
  }
  yield();
  static unsigned long last_ping = millis();
  if ((millis() - last_ping) > PING_TIME) {
    publish_ack("MAIN:ping");
    last_ping = millis();
  }
}
