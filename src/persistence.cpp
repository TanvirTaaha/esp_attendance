/**
 * @file persistence.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Implementations of basic read-write EEPROM functions
 * @version 0.1
 * @date 2025-01-28
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_attendance.h"
#include "mqtt_defines.h"

void eeprom_setup() {
  LOG_DEBUG("in eeprom_setup()");
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("failed to init EEPROM");
  }
}

void eeprom_write_creds() {
  EEPROM.writeBytes(EEPROM_CREDS_ADDR, &credential_struct, sizeof(CredentialStruct));
  EEPROM.commit();
  LOG_DEBUG("Writing credentials to EEPROM");
}

void eeprom_read_creds() {
  EEPROM.readBytes(EEPROM_CREDS_ADDR, (void*)&credential_struct, sizeof(CredentialStruct));
}

// writing byte-by-byte to EEPROM
void eeprom_wirte_str(char *buff, int len, int strart_addr) {
  int end = min(len, EEPROM_SIZE - strart_addr);
  for (int i = 0; i < end; i++) {
    EEPROM.write(strart_addr++, buff[i]);
  }
  EEPROM.commit();
  LOG_WARN("EEPROM write: len will overflow");
}

// reading byte-by-byte from EEPROM
String eeprom_read_string(int addr) {
  String ret;
  for (int i = 0; i < EEPROM_SIZE; i++) {
    byte readValue = EEPROM.read(i);
    // If null character
    if (readValue == 0) {
      break;
    }

    ret += String((char)readValue);
  }
  return ret;
}

void eeprom_write_volume(float vol) {
  EEPROM.writeFloat(EEPROM_VOLUME_LEVEL_ADDR, vol);
  EEPROM.commit();
  LOG_DEBUG("Written vol:%f to EEPROM", vol);
}

float eeprom_read_volume() {
  float f = EEPROM.readFloat(EEPROM_VOLUME_LEVEL_ADDR);
  LOG_DEBUG("Reading vol:%f from EEPROM", f);
  return f;
}