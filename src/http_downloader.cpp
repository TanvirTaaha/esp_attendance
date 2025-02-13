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
// Structure to hold chunk data
struct DataChunk {
  size_t length;
  bool isLast;
  uint8_t buffer[1024];
};

// Queue handle for passing chunks between tasks
static QueueHandle_t chunkQueue;
// Semaphore for signaling CRC completion
static SemaphoreHandle_t crcDoneSemaphore;
// Global variable to store final CRC
static uint32_t finalChecksum;

// CRC processing task
void crcProcessingTask(void* parameter) {
  uint32_t runningCrc = 0xffffffff;
  DataChunk chunk;

  while (true) {
    if (xQueueReceive(chunkQueue, &chunk, portMAX_DELAY) == pdTRUE) {
      // Process the chunk
      for (int i = 0; i < chunk.length; i++) {
        runningCrc = (runningCrc >> 8) ^ crc32_table[(runningCrc & 0xFF) ^ chunk.buffer[i]];
      }

      // If this was the last chunk, finalize CRC and signal completion
      if (chunk.isLast) {
        finalChecksum = ~runningCrc;
        xSemaphoreGive(crcDoneSemaphore);
        runningCrc = 0xffffffff;  // Reset for next potential download
      }
      yield();
    }
  }
}

bool downloadAndVerify(uint32_t expectedChecksum) {
  // Initialize queue and semaphore if not already done
  if (chunkQueue == NULL) {
    chunkQueue = xQueueCreate(5, sizeof(DataChunk));
  }
  if (crcDoneSemaphore == NULL) {
    crcDoneSemaphore = xSemaphoreCreateBinary();
  }

  // Start CRC processing task if not already running
  static TaskHandle_t crcTaskHandle = NULL;
  if (crcTaskHandle == NULL) {
    xTaskCreate(
        crcProcessingTask,
        "CRC_Task",
        4096,
        NULL,
        0,  // the background core, 1 is the core for setup and loop
        &crcTaskHandle);
  }

  HTTPClient http;
  bool success = false;
  DataChunk chunk;

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
    size_t totalBytesRead = 0;
    if (buffer_mp3 == nullptr) {
      buffer_mp3 = (uint8_t*)malloc(max_mp3_buffer_size);
    }
    while (http.connected() && (totalBytesRead < http.getSize())) {
      size_t bytesAvailable = stream->available();

      if (bytesAvailable) {
        yield();
        size_t bytesToRead = min(bytesAvailable, sizeof(chunk.buffer));
        chunk.length = stream->readBytes(chunk.buffer, bytesToRead);
        chunk.isLast = false;
        yield();
        memcpy(buffer_mp3 + totalBytesRead, chunk.buffer, chunk.length);
        totalBytesRead += chunk.length;
        yield();

        // Send chunk to processing task
        xQueueSend(chunkQueue, &chunk, portMAX_DELAY);
      }
      yield();
    }

    // Send final chunk to signal completion
    chunk.length = 0;
    chunk.isLast = true;
    xQueueSend(chunkQueue, &chunk, portMAX_DELAY);
    yield();
    // Wait for CRC processing to complete
    if (xSemaphoreTake(crcDoneSemaphore, pdMS_TO_TICKS(5000)) == pdTRUE) {
      success = (finalChecksum == expectedChecksum);
      LOG_DEBUG("Total bytes read: %d\n", totalBytesRead);
      LOG_DEBUG("Calculated checksum: 0x%08x\n", finalChecksum);
      LOG_DEBUG("Expected checksum: 0x%08x\n", expectedChecksum);
      LOG_INFO("Checksum verification: %s\n", success ? "PASSED" : "FAILED");
      yield();
      if (success)
        publish_ack("HTTP:received and checksum matched");
      else
        publish_ack("HTTP:received but checksum mismatched");
    } else {
      LOG_ERROR("Timeout waiting for CRC calculation");
      publish_ack("HTTP:Timeout waiting for CRC calculation");
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