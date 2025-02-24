/**
 * @file wifiman_advanced.cpp
 * @author Tanvir Hossain Taaha (tanvir.taaha@gmail.com)
 * @brief Manages over the air update of wifi credentials
 * @version 0.1
 * @date 2025-01-26
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "esp_attendance.h"
#include "mqtt_defines.h"

#define TRIGGER_PIN 0

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false;  // change to true to use non blocking

WiFiManager wm;                     // global wm instance
WiFiManagerParameter custom_field;  // global param ( for non blocking w params )

// variable to store clicking time
volatile unsigned long pressDownTime = 0;
bool should_reset_wifiman = false;
bool is_button_active = false;

void saveParamCallback();
void checkButton();
String getParam(String name);

void IRAM_ATTR isr_trigger() {
  bool current_state = digitalRead(TRIGGER_PIN);
  unsigned long current_time = millis();
  if (current_state == LOW || !is_button_active) {
    pressDownTime = current_time;
    is_button_active = true;
  } else {
    if (current_time - pressDownTime > 3000) {
      should_reset_wifiman = true;
      is_button_active = false;
    }
  }
}

// void IRAM_ATTR isr_trigger_up() {
//   digitalWrite(LED_BUILTIN, LOW);
//   if ((millis() - pressDownTime) > 3000) {
//     should_reset_wifiman = true;
//   }
// }

void wifiman_setup() {
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
#ifdef ACTIVATE_LOGGING
  Serial.setDebugOutput(true);
#endif
  delay(3000);
  Serial.println("\n Starting WifiMan");

  attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), isr_trigger, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), isr_trigger_down, RISING);
  // wm.resetSettings();  // wipe settings

  if (wm_nonblocking)
    wm.setConfigPortalBlocking(false);

  // add custom input fields
  const char *custom_html =
      "<br/>"
      "<label for='device_id'>Device ID</label>"
      "<input type='number' name='device_id' placeholder='Enter Device ID'><br/>"
      "<label for='device_pass'>Device Password</label>"
      "<input type='password' name='device_pass' placeholder='Enter Device Password'><br/>"
      "<label for='mqtt_username'>MQTT Username</label>"
      "<input type='text' name='mqtt_username' placeholder='Enter MQTT Username'><br/>"
      "<label for='mqtt_pass'>MQTT Password</label>"
      "<input type='password' name='mqtt_pass' placeholder='Enter MQTT Password'><br/>";

  new (&custom_field) WiFiManagerParameter(custom_html);

  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  //
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  // set static ip
  //  wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  //  wm.setShowStaticFields(true); // force show static ip fields
  //  wm.setShowDnsFields(true);    // force show dns field always

  // wm.setConnectTimeout(20); // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(600);  // auto close configportal after n seconds
  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons

  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("ESP", "password");  // password protected ap

  if (!res) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } else {
    // if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    // connectToMqtt();
  }
}

// void checkButton() {
//   // check for button press
//   if (digitalRead(TRIGGER_PIN) == LOW) {
//     // poor mans debounce/press-hold, code not ideal for production
//     delay(50);
//     if (digitalRead(TRIGGER_PIN) == LOW) {
//       Serial.println("Button Pressed");
//       // still holding button for 3000 ms, reset settings, code not ideaa for production
//       delay(3000);  // reset delay hold
//       if (digitalRead(TRIGGER_PIN) == LOW) {
//         Serial.println("Button Held");
//         Serial.println("Erasing Config, restarting");
//         wm.resetSettings();
//         ESP.restart();
//       }

//       // start portal w delay
//       Serial.println("Starting config portal");
//       wm.setConfigPortalTimeout(120);

//       if (!wm.startConfigPortal("OnDemandAP", "password")) {
//         Serial.println("failed to connect or hit timeout");
//         delay(3000);
//         // ESP.restart();
//       } else {
//         // if you get here you have connected to the WiFi
//         Serial.println("connected...yeey :)");
//       }
//     }
//   }
// }

String getParam(String name) {
  // read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback() {
  // LOG_DEBUG("[CALLBACK] saveParamCallback fired");
  // LOG_DEBUG("PARAM device_id = %s", getParam("device_id").c_str());
  // LOG_DEBUG("PARAM device_pass = %s", getParam("device_pass").c_str());
  // LOG_DEBUG("PARAM mqtt_username = %s", getParam("mqtt_username").c_str());
  // LOG_DEBUG("PARAM mqtt_pass = %s", getParam("mqtt_pass").c_str());

  credential_struct.device_id = getParam("device_id").toInt();
  strcpy(credential_struct.device_pass, getParam("device_pass").c_str());
  strcpy(credential_struct.mqtt_username, getParam("mqtt_username").c_str());
  strcpy(credential_struct.mqtt_pass, getParam("mqtt_pass").c_str());
  eeprom_write_creds();
}

void wifiman_loop() {
  if (should_reset_wifiman) {
    should_reset_wifiman = false;
    Serial.println("Button Held");
    Serial.println("Erasing Config...");
    wm.resetSettings();  // wipe settings
    delay(1000);
    Serial.println("Erased Config, restarting");
    ESP.restart();
  }
}
