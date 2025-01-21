#include "esp_attendance.h"



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while(!Serial && millis() < 2000) Serial.print(".");
  Serial.println();
#ifdef ATTENDANCE_DEBUG
    Serial.println("Serial Started");
#endif

  async_mqtt_setup();
#ifdef ATTENDANCE_DEBUG
    Serial.println("MQTT started.");
#endif

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long last_millis = millis();
  if ( (millis() - last_millis) > 5000) {
    Serial.println("main loop.");
    last_millis = millis();
  }
}
