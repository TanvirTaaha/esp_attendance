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

#include "HTTPClient.h"
#include "crc_lookup.h"
#include "esp_attendance.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

// Structure to hold chunk data
struct DataChunk {
  uint8_t buffer[1024];
  size_t length;
  bool isLast;
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
      while (chunk.length--) {
        runningCrc = (runningCrc >> 8) ^ crc32_table[(runningCrc & 0xFF) ^ chunk.buffer[chunk.length]];
      }

      // If this was the last chunk, finalize CRC and signal completion
      if (chunk.isLast) {
        finalChecksum = ~runningCrc;
        xSemaphoreGive(crcDoneSemaphore);
        runningCrc = 0xffffffff;  // Reset for next potential download
      }
    }
  }
}

bool downloadAndVerify(const char* url, uint32_t expectedChecksum) {
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
        1,
        &crcTaskHandle);
  }

  HTTPClient http;
  bool success = false;
  DataChunk chunk;

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    size_t totalBytesRead = 0;
    if (http.getSize() > max_mp3_buffer_size) {
      
    }
    while (http.connected() && (totalBytesRead < http.getSize())) {
      size_t bytesAvailable = stream->available();

      if (bytesAvailable) {
        size_t bytesToRead = min(bytesAvailable, sizeof(chunk.buffer));
        chunk.length = stream->readBytes(chunk.buffer, bytesToRead);
        chunk.isLast = false;
        memcpy(buffer_mp3 + totalBytesRead, chunk.buffer, chunk.length);
        totalBytesRead += chunk.length;

        // Send chunk to processing task
        xQueueSend(chunkQueue, &chunk, portMAX_DELAY);
      }
      yield();
    }

    // Send final chunk to signal completion
    chunk.length = 0;
    chunk.isLast = true;
    xQueueSend(chunkQueue, &chunk, portMAX_DELAY);

    // Wait for CRC processing to complete
    if (xSemaphoreTake(crcDoneSemaphore, pdMS_TO_TICKS(5000)) == pdTRUE) {
      success = (finalChecksum == expectedChecksum);

      Serial.printf("Total bytes read: %d\n", totalBytesRead);
      Serial.printf("Calculated checksum: 0x%08x\n", finalChecksum);
      Serial.printf("Expected checksum: 0x%08x\n", expectedChecksum);
      Serial.printf("Checksum verification: %s\n", success ? "PASS" : "FAIL");
    } else {
      Serial.println("Timeout waiting for CRC calculation");
    }
  }

  http.end();
  return success;
}