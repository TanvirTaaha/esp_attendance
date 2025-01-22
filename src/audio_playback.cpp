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


BufferRTOS<uint8_t> bufferRTOS(buffer_size_base64);
QueueStream<uint8_t> queue(bufferRTOS);
I2SStream i2s;

void audio_init() {
  queue.begin();
}