/**
 * @file esp_attendance.h
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Contains all the includes.
 * @version 0.1
 * @date 2025-01-21
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once
#ifndef __ESP_ATTENDANCE_H__
#define __ESP_ATTENDANCE_H__

#include <Arduino.h>
#include <mbedtls/base64.h>
// #include <AsyncMQTT_ESP32.h>

// AsyncMQTT
#include <WiFi.h>

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

// mqtt funcs
void async_mqtt_setup();

#endif