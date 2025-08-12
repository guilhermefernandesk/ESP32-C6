#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

  WiFiManager wm;

  wm.resetSettings();

  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("connected...yeey :)");
  }
}

void loop() {
}
