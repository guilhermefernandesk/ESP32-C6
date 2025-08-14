/*********************************************************
 * Projeto: Controle MQTT com ESP32-C6 + WifiManager
 *
 * Descrição: Este projeto permite publicar e receber mensagens MQTT
 *            para controle de um LED via broker público.
 *            Também utiliza WifiManager para gerenciar as configurações de Wi-Fi
 *            e broker MQTT.
 *
 * Autor: Guilherme Fernandes
 * Data: 14/08/2025
 * Placa: ESP32-C6 DevKitC-1
 *********************************************************/

#include <WiFi.h>          // Inclui a biblioteca para Wi-Fi
#include <PubSubClient.h>  // Inclui a biblioteca para MQTT
#include <WiFiManager.h>   // Inclui a biblioteca do WiFiManager (https://github.com/tzapu/WiFiManager)
#include <WebServer.h>     // Inclui a biblioteca WebServer para criar um servidor web
#include "html_page.h"     // Inclui o arquivo HTML que contém a página web

// Definição dos pinos
#define LED_VERMELHO 18
#define BOTAO 19

// Definicao de variaveis
#define topic_led "esp32/led/vermelho"   // Tópico MQTT para o LED Vermelho
#define broker_default "broker.emqx.io"  // Endereço do padrão para o Broker MQTT
#define mqtt_tcp_port_default "1883"     // Porta do Broker MQTT TCP
#define mqtt_ws_port_default "8084"      // Porta do Broker MQTT WebSocket
#define id_mqtt "esp32c6_guilherme"      // ID do cliente MQTT - Pode ser alterado, mas deve ser único para cada cliente conectado ao broker

// Mapeamento dos pinos
const uint8_t LED_PIN = LED_VERMELHO;
const uint8_t BUTTON_PIN = BOTAO;

bool statusLED = LOW;  // Status do LED  (LOW = Desligado / HIGH = Ligado)

WiFiClient espClient;                // Cria um cliente Wi-Fi
PubSubClient mqttClient(espClient);  // Cria um cliente MQTT

// Criação do servidor WebSocket
WebServer server(80);

// Definição dos Tópicos MQTT
const char* topic_led_vermelho = topic_led;

// Configuração do Broker MQTT
const char* idMqtt = id_mqtt;
char mqtt_server[40] = broker_default;
char mqtt_port[6] = mqtt_tcp_port_default;
char mqtt_ws_port[6] = mqtt_ws_port_default;

// global wm instance
WiFiManager wm;

// Criação dos parâmetros do WiFiManager
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);

// Função para definir o estado do LED
void setLED(bool state) {
  statusLED = state;
  digitalWrite(LED_PIN, statusLED);
  Serial.println("LED: " + String(statusLED ? "ON" : "OFF"));
}

// Callback para processar mensagens recebidas do MQTT
void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";                    // Cria uma string para armazenar a mensagem
  for (int i = 0; i < length; i++) {  // Percorre a mensagem recebida
    msg += (char)message[i];          // Concatena os caracteres da mensagem
  }

  // Imprime a mensagem recebida no monitor serial - Log
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Conteúdo: ");
  Serial.println(msg);

  // Verifica se a mensagem é para o LED vermelho
  if (strcmp(topic, topic_led_vermelho) == 0) {  // Compara o tópico recebido com o tópico do LED Vermelho
    if (msg == "ON") {
      setLED(true);
    } else if (msg == "OFF") {
      setLED(false);
    }
  }
}

// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi Conectado, IP:");
  Serial.println(WiFi.localIP());
}

// Função para alterar as configurações do MQTT
void changeMQTT(String mqttServer) {
  strncpy(mqtt_server, mqttServer.c_str(), sizeof(mqtt_server) - 1);
  mqtt_server[sizeof(mqtt_server) - 1] = '\0'; // garante término correto
  mqttClient.disconnect();
}

// Função de conexao ao Broker MQTT
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Conectando ao broker: " + String(mqtt_server) + ":" + String(mqtt_port));
    mqttClient.setServer(mqtt_server, atoi(mqtt_port));

    if (mqttClient.connect(idMqtt)) {
      Serial.println("Conectado!");

      // Assinar os tópicos dos LEDs
      mqttClient.subscribe(topic_led_vermelho);

      // Imprime a mensagem no monitor serial
      Serial.println("Assinado nos tópicos: ");
      Serial.println(topic_led_vermelho);
    } else {
      Serial.print("Falha, código: ");     // Imprime a mensagem no monitor serial
      Serial.println(mqttClient.state());  // Imprime o código de erro no monitor serial
      delay(5000);                         // Aguarda 5 segundos e tenta novamente
    }
  }
}

// Rotas
void handleStatus() {
  server.send(200, "text/plain", "LED=" + String(statusLED ? "ON" : "OFF") + ";mqttServer=" + String(mqtt_server) + ";mqttPort=" + String(mqtt_ws_port) + ";topic=" + String(topic_led_vermelho));
  Serial.println("Status do LED: ");
  Serial.println(statusLED ? "ON" : "OFF");  // Imprime o status do LED
}

void handleRoot() {
  Serial.println("Acessando a página principal");
  server.send(200, "text/html", index_html);  // Retorna o HTML para o cliente
}

void handleNotFound() {
  Serial.println("Página não encontrada");
  server.send(404, "text/plain", "Not found");  // Retorna uma resposta 404 Not Found
}

void handleSave() {
  Serial.println("Salvando configurações");
  changeMQTT(server.arg("mqttServer"));
  server.send(200, "text/plain", "Configurações salvas");
}

/*-------------------------------------------------------------------------------
  Tarefa para ler o estado do botão e publicar a mensagem no tópico MQTT
 *---------------------------------------------------------------------------*/
void taskBotao(void* parameter) {
  for (;;) {
    if (digitalRead(BUTTON_PIN) == LOW) {  // Push button pressed
      // Key debounce handling
      delay(100);
      int startTime = millis();
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);
      }
      int endTime = millis();
      // If key pressed for more than 3secs, reset settings
      if ((endTime - startTime) > 3000) {
        Serial.printf("Reset settings.\n");
        Serial.println("Erasing Config, restarting");
        wm.resetSettings();
        ESP.restart();
      } else if ((endTime - startTime) > 10000) {  // If key pressed for more than 10sec, start config portal
        // start portal w delay
        Serial.println("Starting config portal");
        wm.setConfigPortalTimeout(120);

        if (!wm.startConfigPortal("OnDemandAP", "password")) {
          Serial.println("failed to connect or hit timeout");
        } else {
          Serial.println("connected...yeey :)");
        }
      } else {
        statusLED = !statusLED;  // Inverte o estado do LED
        String msg = statusLED ? "ON" : "OFF";
        mqttClient.publish(topic_led_vermelho, msg.c_str(), false);  // Publica a mensagem no tópico do LED Vermelho
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));  // Aguarda 100 ms antes de verificar novamente
  }
}


// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// WiFiManagerParameter custom_field; // global param ( for non blocking w params )

// String getParam(String name) {
//   // read parameter from server, for customhmtl input
//   String value;
//   if (wm.server->hasArg(name)) {
//     value = wm.server->arg(name);
//   }
//   return value;
// }

// void saveParamCallback() {
//   Serial.println("[CALLBACK] saveParamCallback fired");
//   Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
// }

// void bindServerCallback() { wm.server->on("/custom", handleRoute); }

// void handleRoute() {
//   Serial.println("[HTTP] handle custom route");
//   wm.server->send(200, "text/plain", "hello from user code");
// }

void setup() {
  Serial.begin(115200);
  Serial.println("\n Starting");

  // Configuração dos LEDs como saída e desliga os LEDs
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // Configura o pino do botão como entrada com pull-up interno
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Criando a Task para leitura e envio do botao
  xTaskCreate(
    taskBotao,     // Nome da função da task
    "Task Botao",  // Nome descritivo
    4096,          // Tamanho da stack
    NULL,          // Parâmetro (não usado)
    1,             // Prioridade da task
    NULL           // Handle (não necessário aqui)
  );

  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);

  // const char *bufferStr = R"(
  // <!-- INPUT CHOICE -->
  // <br/>
  // <p>Select Choice</p>
  // <input style='display: inline-block;' type='radio' id='choice1' name='program_selection' value='1'>
  // <label for='choice1'>Choice1</label><br/>
  // <input style='display: inline-block;' type='radio' id='choice2' name='program_selection' value='2'>
  // <label for='choice2'>Choice2</label><br/>

  // <!-- INPUT SELECT -->
  // <br/>
  // <label for='input_select'>Label for Input Select</label>
  // <select name="input_select" id="input_select" class="button">
  // <option value="0">Option 1</option>
  // <option value="1" selected>Option 2</option>
  // <option value="2">Option 3</option>
  // <option value="3">Option 4</option>
  // </select>
  // )";

  // WiFiManagerParameter custom_html_inputs(bufferStr);

  // set custom html menu content , inside menu item "custom", see setMenu()
  // const char *menuhtml = "<form action='/custom' method='get'><button>Custom</button></form><br/>\n";
  // wm.setCustomMenuHTML(menuhtml);

  // add a custom input field
  // int customFieldLength = 40;

  // test custom html(radio)
  // const char *custom_radio_str =
  //     "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' "
  //     "checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' "
  //     "name='customfieldid' value='3'> Three";

  // new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input

  // add all your parameters here
  // wm.addParameter(&custom_field);
  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  // wm.addParameter(&custom_html_inputs);

  // wm.setWebServerCallback(bindServerCallback);

  // wm.setSaveParamsCallback(saveParamCallback);

  // set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);

  // custom menu via array or vector
  // std::vector<const char *> menu = {"wifi", "info", "param", "custom", "sep", "restart", "exit"};
  // wm.setMenu(menu);

  wm.setConfigPortalTimeout(120);  // auto close configportal after n seconds

  if (!wm.autoConnect("AutoConnectAP", "password")) {
    Serial.println("Failed to connect");
  } else {
    Serial.println("connected...yeey :)");
  }

  // read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  Serial.println("\tmqtt_server : " + String(mqtt_server));
  Serial.println("\tmqtt_port : " + String(mqtt_port));

  mqttClient.setCallback(callback);  // Define a função de callback
  connectMQTT();                     // Conecta ao Broker MQTT

  // Definicao de Rotas do Servidor (como um Backend)
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/save", handleSave);
  server.onNotFound(handleNotFound);

  server.begin();  // Inicia o servidor web
  Serial.println("Servidor iniciado");

  // save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Salvando...");
  }
}

void loop() {
  // Se a conexão com o Broker MQTT foi perdida, reconecta
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();  // Mantém a conexão com o Broker MQTT

  server.handleClient(); // Processa as requisições do cliente
}
