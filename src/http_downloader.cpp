/**
 * @file http_downloader.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Implementations of downloading and checking for audio files over http
 * @version 0.1
 * @date 2025-02-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "crc_lookup.h"
#include "esp_attendance.h"
#include "mqtt_defines.h"

// CRC calculation in onepass
uint32_t calculateCRC32(const uint8_t* data, size_t length) {
  uint32_t crc = 0xffffffff;
  for (size_t i = 0; i < length; i++) {
    crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ data[i]];
  }
  return ~crc;
}

inline uint32_t calc_sum(uint8_t* data, size_t length) {
  int sum = 0;
  for (size_t i = 0; i < length; i++) {
    sum += data[i];
  }
  return sum;
}

bool downloadAndVerify(uint32_t expectedChecksum) {
  HTTPClient http;
  bool success = false;
  http.setReuse(true);

  char url[100];
  sprintf(url, "http://%d.%d.%d.%d:8000/potpot?device_id=%d&msg_id=%d\0", MQTT_HOST[0], MQTT_HOST[1], MQTT_HOST[2], MQTT_HOST[3], credential_struct.device_id, mqtt_payload_struct.msg_id);

  yield();
  http.begin(url);
  int httpCode = http.GET();
  yield();

  if (httpCode == HTTP_CODE_OK) {
    if (http.getSize() > max_mp3_buffer_size) {
      LOG_ERROR("File size exceeds buffer size");
      publish_ack("HTTP:File size exceeds buffer size");
      return false;
    }
    if (http.getSize() != mqtt_payload_struct.file_size) {
      LOG_ERROR("File size mismatch with mqtt");
      publish_ack("HTTP:File size mismatch with mqtt payload");
      return false;
    }
    yield();
    WiFiClient* stream = http.getStreamPtr();
    yield();
    if (buffer_mp3 == nullptr) {
      buffer_mp3 = (uint8_t*)malloc(max_mp3_buffer_size);
      yield();
    }

    size_t totalBytesRead = stream->readBytes(buffer_mp3, http.getSize());
    yield();

    // uint32_t checksum = calculateCRC32(buffer_mp3, totalBytesRead);
    uint32_t checksum = calc_sum(buffer_mp3, totalBytesRead);
    yield();
    success = (expectedChecksum == checksum);
    LOG_DEBUG("Total bytes read: %d\n", totalBytesRead);
    LOG_DEBUG("Calculated checksum: 0x%08x\n", checksum);
    LOG_DEBUG("Expected checksum: 0x%08x\n", expectedChecksum);
    LOG_INFO("Checksum verification: %s\n", success ? "PASSED" : "FAILED");
    if (success) {
      publish_ack("HTTP:received and checksum matched");
    } else {
      publish_ack("HTTP:received but checksum mismatched");
    }

  } else {
    yield();
    LOG_ERROR("HTTP GET failed, error: %d\n", httpCode);
    String error_msg = "HTTP:GET failed, error: " + String(httpCode);
    publish_ack(error_msg.c_str());
  }
  yield();
  http.end();
  return success;
}

void setup_http() {
  // Initialize with longer timeout
  esp_task_wdt_init(10, true);

  // Optional: Add specific tasks to watchdog
  TaskHandle_t asyncTcpTask = xTaskGetHandle("async_tcp");
  if (asyncTcpTask != NULL) {
    esp_task_wdt_add(asyncTcpTask);
  }
}