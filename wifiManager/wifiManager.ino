#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager

#define TRIGGER_PIN 9

//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[40];
char mqtt_port[6] = "8080";
char api_token[34] = "YOUR_API_TOKEN";

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}



WiFiManagerParameter custom_field;  // global param ( for non blocking w params )

// The extra parameters to be configured (can be either global or just in the setup)
// After connecting, parameter.getValue() will get you the configured value
// id/name placeholder/prompt default length
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_api_token("apikey", "API token", api_token, 32);

WiFiManager wm;  // global wm instance


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

void bindServerCallback() { wm.server->on("/custom", handleRoute); }

void handleRoute() {
  Serial.println("[HTTP] handle custom route");
  wm.server->send(200, "text/plain", "hello from user code");
}

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.println("\n Starting");
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  const char *bufferStr = R"(
  <!-- INPUT CHOICE -->
  <br/>
  <p>Select Choice</p>
  <input style='display: inline-block;' type='radio' id='choice1' name='program_selection' value='1'>
  <label for='choice1'>Choice1</label><br/>
  <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
  <label for='choice2'>Choice2</label><br/>

  <!-- INPUT SELECT -->
  <br/>
  <label for='input_select'>Label for Input Select</label>
  <select name="input_select" id="input_select" class="button">
  <option value="0">Option 1</option>
  <option value="1" selected>Option 2</option>
  <option value="2">Option 3</option>
  <option value="3">Option 4</option>
  </select>
  )";

  WiFiManagerParameter custom_html_inputs(bufferStr);
  // set custom html menu content , inside menu item "custom", see setMenu()
  const char *menuhtml = "<form action='/custom' method='get'><button>Custom</button></form><br/>\n";
  wm.setCustomMenuHTML(menuhtml);

  // add a custom input field
  int customFieldLength = 40;
  // test custom html(radio)
  const char *custom_radio_str =
    "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' "
    "checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' "
    "name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str);  // custom html input
  wm.addParameter(&custom_field);
  //add all your parameters here
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_api_token);
  wm.addParameter(&custom_html_inputs);
  wm.setWebServerCallback(bindServerCallback);

  wm.setSaveParamsCallback(saveParamCallback);
  //set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // custom menu via array or vector
  std::vector<const char *> menu = { "wifi", "info", "param", "custom", "sep", "restart", "exit" };
  wm.setMenu(menu);

  wm.setConfigPortalTimeout(30);  // auto close configportal after n seconds

  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("connected...yeey :)");
  }

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(api_token, custom_api_token.getValue());
  Serial.println("The values in the file are: ");
  Serial.println("\tmqtt_server : " + String(mqtt_server));
  Serial.println("\tmqtt_port : " + String(mqtt_port));
  Serial.println("\tapi_token : " + String(api_token));

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Salvando...");
  }
}

void loop() {
  if (digitalRead(TRIGGER_PIN) == LOW) {  // Push button pressed
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
