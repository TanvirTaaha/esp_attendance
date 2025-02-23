#include "esp_attendance.h"

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;      // GMT offset in seconds (change based on your timezone)
const int daylightOffset_sec = 0;  // Daylight savings offset (if applicable)

// Global time variables
struct tm timeinfo;
time_t bootTime;
bool timeInitialized = false;

void setupTime() {
  // Configure time with NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Wait for time to be set
  int retry = 0;
  while (!getLocalTime(&timeinfo) && retry < 10) {
    LOG_ERROR("Failed to obtain time");
    delay(500);
    retry++;
  }

  if (retry < 10) {
    time(&bootTime);  // Save boot time
    timeInitialized = true;

    LOG_INFO("Time synchronized!");
    LOG_INFO("Boot time: %s", ctime(&bootTime));
  } else {
    LOG_ERROR("Could not synchronize time");
  }
}

// Function to get seconds elapsed since boot
long getSecondsSinceBoot() {
  if (!timeInitialized) return 0;

  time_t now;
  time(&now);
  return difftime(now, bootTime);
}

// Function to get formatted time string
String getFormattedTime() {
  if (!timeInitialized) return "Time not initialized";

  char timeString[30];
  time_t now;
  time(&now);
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return String(timeString);
}
