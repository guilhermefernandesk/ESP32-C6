/*********************************************************
 * Projeto: Controle MQTT com ESP32-C6 + FreeRTOS
 *
 * Descrição: Este projeto permite publicar e receber mensagens MQTT
 *            para controle de um LED via broker público.
 *            Também utiliza FreeRTOS para tratar eventos de botão
 *            de forma assíncrona.
 *
 * Autor: Guilherme Fernandes
 * Data: 14/07/2025
 * Placa: ESP32-C6 DevKitC-1
 *********************************************************/

#include <WiFi.h>               // Inclui a biblioteca para Wi-Fi
#include <PubSubClient.h>       // Inclui a biblioteca para MQTT
#include <button.h>             // Inclui a biblioteca para manipulação de botões
#include <ESPAsyncWebServer.h>  // Inclui a biblioteca para servidor web assíncrono
#include <AsyncTCP.h>           // Inclui a biblioteca para TCP assíncrono
#include "html_page.h"          // Inclui o arquivo HTML que contém a página web

// Definição dos pinos
#define LED_VERMELHO 18
#define BOTAO 19

// Definicao de variaveis
#define topic_led "esp32/led/vermelho"  // Tópico MQTT para o LED Vermelho
#define broker_default "broker.emqx.io" // Endereço do Broker MQTT - Se desejar utilizar outro, substitua este pelo endereço do seu broker
#define mqtt_tcp_port_default 1883      // Porta do Broker MQTT TCP
#define mqtt_ws_port_default 8084       // Porta do Broker MQTT WebSocket
#define id_mqtt "esp32c6_guilherme"     // ID do cliente MQTT - Pode ser alterado, mas deve ser único para cada cliente conectado ao broker

// Mapeamento dos pinos
const uint8_t LED_PIN = LED_VERMELHO;  
const uint8_t BUTTON_PIN = BOTAO;  

Button button(BUTTON_PIN);     // Inicializa o botao com input pull-up

WiFiClient espClient;          // Cria um cliente Wi-Fi
PubSubClient mqtt(espClient);  // Cria um cliente MQTT

// Criação do servidor WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Configuração Wi-Fi
const char* ssid = "*****";       // SSID da rede Wi-Fi substituir pelo SSID da sua rede
const char* password = "******";  // Senha da rede Wi-Fi substituir pela senha da sua rede

// Definição dos Tópicos MQTT
const char* topic_led_vermelho = topic_led;

// Configuração do Broker MQTT
const int mqtt_port = mqtt_tcp_port_default;
const char* idMqtt = id_mqtt;
String mqtt_server = broker_default;
int mqtt_ws_port = mqtt_ws_port_default;

// Variável para armazenar o estado do LED
bool ledState = LOW;  

// Protótipos das funções
void taskBotao(void* parameter);                                 
void callback(char* topic, byte* message, unsigned int length);

// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("\nWiFi Conectado!"); 
  Serial.print("Endereço de IP: ");     
  Serial.println(WiFi.localIP());  
}

// Evento WiFi Desconectado
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("Desconectado do WiFi");
  Serial.print("WiFi perdeu a conexão. Razão: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Tentando reconectar");
  WiFi.begin(ssid, password);
}

// Função para lidar com a página principal websocket
void handleRoot(AsyncWebServerRequest *request) {
  Serial.println("Acessando a página principal");
  request->send(200, "text/html", index_html); 
}

// Função de configuração do Wi-Fi
void setupWiFi() {
  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.begin(ssid, password);              // Conecta-se à rede Wi-Fi
  Serial.print("Conectando ao WiFi...");   // Aguarda a conexão com a rede
  while (WiFi.status() != WL_CONNECTED) {  // Enquanto não estiver conectado, aguarda
    delay(500);                            // Aguarda 500ms
    Serial.print(".");                     // Imprime um ponto no monitor serial
  }
}

// Função de conexao ao Broker MQTT
void connectMQTT() {
  while (!mqtt.connected()) {                                                        
    Serial.println("Conectando ao broker: " + mqtt_server + ":" + String(mqtt_port));
    mqtt.setServer(mqtt_server.c_str(), mqtt_port); 

    if (mqtt.connect(idMqtt)) {
      Serial.println("Conectado!");

      // Assinar os tópicos dos LEDs
      mqtt.subscribe(topic_led_vermelho);

      // Imprime a mensagem no monitor serial
      Serial.println("Assinado nos tópicos: ");
      Serial.println(topic_led_vermelho);
    } else {
      Serial.print("Falha, código: ");  // Imprime a mensagem no monitor serial
      Serial.println(mqtt.state());     // Imprime o código de erro no monitor serial
      delay(5000);                      // Aguarda 5 segundos e tenta novamente
    }
  }
}

// Função para definir o estado do LED
void setLED(bool state) {
  ledState = state;
  digitalWrite(LED_PIN, ledState);
  Serial.println("LED: " + String(ledState ? "ON" : "OFF"));
}

// Callback para processar mensagens recebidas do MQTT
void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";                   // Cria uma string para armazenar a mensagem
  for (int i = 0; i < length; i++){  // Percorre a mensagem recebida
    msg += (char)message[i];         // Concatena os caracteres da mensagem
  }

  // Imprime a mensagem recebida no monitor serial - Log
  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Conteúdo: ");
  Serial.println(msg);

  // Verifica se a mensagem é para o LED vermelho
  if (strcmp(topic, topic_led_vermelho) == 0){  // Compara o tópico recebido com o tópico do LED Vermelho
      if (msg == "ON") {
        setLED(true);
      } else if (msg == "OFF") {
        setLED(false);
      }
  }
}

// Função para lidar com eventos do WebSocket
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%lu connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      client->text("mqtt_server=" + mqtt_server + ";mqtt_port=" + String(mqtt_ws_port) + ";topic_led_vermelho=" + String(topic_led_vermelho));
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%lu disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      {
        String msg = "";
        for (size_t i = 0; i < len; i++) {
          msg += (char)data[i];
        }
        Serial.println("Mensagem recebida via WebSocket: " + msg);
        mqtt_server = msg.substring(msg.indexOf("mqtt_server=") + 12, msg.indexOf(";mqtt_port="));
        mqtt_ws_port = msg.substring(msg.indexOf("mqtt_port=") + 10).toInt();

        Serial.println("Novo servidor MQTT: " + mqtt_server);
        Serial.println("Nova porta WebSocket MQTT: " + String(mqtt_ws_port));
        mqtt.disconnect();
        break;
      }
    default:
      break;
  }
}

/*-------------------------------------------------------------------------------
  Tarefa para ler o estado do botão e publicar a mensagem no tópico MQTT
 *---------------------------------------------------------------------------*/
void taskBotao(void* parameter) {
  for (;;) {
    button.update();                                           // Atualiza o estado do botão
    if (button.wasPressed()) {                                 // Verifica se o botão foi pressionado
      Serial.println("Botão pressionado");                     
      ledState = !ledState;                                    // Inverte o estado do LED
      String msg = ledState ? "ON" : "OFF";                    
      mqtt.publish(topic_led_vermelho, msg.c_str(), false);    // Publica a mensagem no tópico do LED Vermelho
    }
    vTaskDelay(pdMS_TO_TICKS(100));                            // Aguarda 100 ms antes de verificar novamente
  }
}

void setup() {
  Serial.begin(115200);  // Inicializa a comunicação serial
  setupWiFi();           // Configura a conexão Wi-Fi

  // Configuração dos LEDs como saída e desliga os LEDs
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  mqtt.setCallback(callback);   // Define a função de callback
  connectMQTT();                // Conecta ao Broker MQTT

  ws.onEvent(onWebSocketEvent);  // Registra o evento do WebSocket
  server.addHandler(&ws);        // Adiciona o WebSocket ao servidor

   // Definicao de Rotas do Servidor (como um Backend)
  server.on("/", HTTP_GET, handleRoot);

  server.begin();  // Inicia o servidor web
  Serial.println("Servidor iniciado");

  // Criando a Task para leitura e envio do botao
  xTaskCreate(
    taskBotao,     // Nome da função da task
    "Task Botao",  // Nome descritivo
    4096,          // Tamanho da stack
    NULL,          // Parâmetro (não usado)
    1,             // Prioridade da task
    NULL           // Handle (não necessário aqui)
  );
}

void loop() {
  // Se a conexão com o Broker MQTT foi perdida, reconecta
  if (!mqtt.connected()) {
    connectMQTT();  
  }
  mqtt.loop();  // Mantém a conexão com o Broker MQTT

  // Limpa conexões WebSocket desconectadas
  ws.cleanupClients();
}
