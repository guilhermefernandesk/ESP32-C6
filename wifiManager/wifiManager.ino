#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#define TRIGGER_PIN 9

WiFiManager wm;                    // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )

String getParam(String name) {
  // read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback() {
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.println("\n Starting");
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  wm.resetSettings();

  // add a custom input field
  int customFieldLength = 40;
  // test custom html(radio)
  const char *custom_radio_str =
      "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' "
      "checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' "
      "name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input
  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
  wm.setMenu(menu);

  wm.setConfigPortalTimeout(30); // auto close configportal after n seconds

  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("connected...yeey :)");
  }
}

void loop() {
  if (digitalRead(TRIGGER_PIN) == LOW) { // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(TRIGGER_PIN) == LOW) {
      delay(50);
    }
    int endTime = millis();
    // If key pressed for more than 3secs, reset settings
    if ((endTime - startTime) > 3000) {
      Serial.printf("Reset settings.\n");
      Serial.println("Erasing Config, restarting");
      wm.resetSettings();
      ESP.restart();
    } else {
      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);

      if (!wm.startConfigPortal("OnDemandAP", "password")) {
        Serial.println("failed to connect or hit timeout");
      } else {
        Serial.println("connected...yeey :)");
      }
    }
  }
}
