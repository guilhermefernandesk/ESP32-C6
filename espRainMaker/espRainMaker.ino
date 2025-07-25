/*
  * ESP RainMaker - Switch Example
  * Adaptado por: Guilherme Fernandes
*/

#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"
#include "AppInsights.h"

const char *service_name = "PROV_1234"; // Nome do serviço de provisionamento
const char *pop = "abcd1234";           // Chave de PoP (Proof of Possession)

static int LED_PIN = 18;    // Pino do LED
static int BUTTON_PIN = 19; // Pino do botão 

bool switch_state = false;  // Estado do switch

static Switch *my_switch = NULL; // Dispositivo do tipo Switch

// WARNING: sysProvEvent is called from a separate FreeRTOS task (thread)!
void sysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_PROV_START:
      Serial.printf("\nProvisioning Started with name \"%s\" and PoP \"%s\" on BLE\n", service_name, pop);
      WiFiProv.printQR(service_name, pop, "ble");
      break;
    case ARDUINO_EVENT_PROV_INIT: WiFiProv.disableAutoStop(10000); break;
    case ARDUINO_EVENT_PROV_CRED_SUCCESS: WiFiProv.endProvision(); break;
    default:;
  }
}

// Funcao de callback chamada quando o dispositivo recebe um valor para o parâmetro
void write_callback(Device *device, Param *param, const param_val_t val, void *priv_data, write_ctx_t *ctx) {
  const char *device_name = device->getDeviceName();
  const char *param_name = param->getParamName();

  if (strcmp(param_name, "Power") == 0) {
    Serial.printf("Received value = %s for %s - %s\n", val.val.b ? "true" : "false", device_name, param_name);
    switch_state = val.val.b;
    (switch_state == false) ? digitalWrite(LED_PIN, LOW) : digitalWrite(LED_PIN, HIGH);
    param->updateAndReport(val);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Node my_node;
  my_node = RMaker.initNode("ESP RainMaker Node");

  // Initialize switch device
  my_switch = new Switch("Switch", &LED_PIN);
  if (!my_switch) {
    return;
  }
  // Standard switch device
  my_switch->addCb(write_callback);

  // Add switch device to the node
  my_node.addDevice(*my_switch);

  RMaker.enableOTA(OTA_USING_TOPICS); // This is optional

  RMaker.enableTZService();

  RMaker.enableSchedule();

  RMaker.enableScenes();

  initAppInsights(); // Enable ESP Insights.

  RMaker.enableSystemService(SYSTEM_SERV_FLAGS_ALL, 2, 2, 2);

  //WiFiProv.initProvision(NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM);

  RMaker.start();

  WiFi.onEvent(sysProvEvent);  // Will call sysProvEvent() from another thread.

  WiFiProv.beginProvision(NETWORK_PROV_SCHEME_BLE, NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM, NETWORK_PROV_SECURITY_1, pop, service_name);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {  // Push button pressed

    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
    }
    int endTime = millis();

    if ((endTime - startTime) > 10000) {
      // If key pressed for more than 10secs, reset all
      Serial.printf("Reset to factory.\n");
      RMakerFactoryReset(2);
    } else if ((endTime - startTime) > 3000) {
      Serial.printf("Reset Wi-Fi.\n");
      // If key pressed for more than 3secs, but less than 10, reset Wi-Fi
      RMakerWiFiReset(2);
    } else {
      // Toggle device state
      switch_state = !switch_state;
      Serial.printf("Toggle State to %s.\n", switch_state ? "true" : "false");
      if (my_switch) {
        my_switch->updateAndReportParam(ESP_RMAKER_DEF_POWER_NAME, switch_state);
      }
      (switch_state == false) ? digitalWrite(LED_PIN, LOW) : digitalWrite(LED_PIN, HIGH);
    }
  }
  delay(100);
}
