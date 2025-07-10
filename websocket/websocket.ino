#include <WiFi.h>               // Inclui a biblioteca WiFi para conectar ao WiFi
#include <ESPAsyncWebServer.h>  // Inclui a biblioteca para servidor web assíncrono
#include <AsyncTCP.h>           // Inclui a biblioteca para TCP assíncrono
#include "html_page.h"          // Inclui o arquivo HTML que contém a página web
#include "credentials.h"        // Inclui o arquivo de credenciais do WiFi
#include <button.h>             // Inclui a biblioteca para manipulação de botões

const uint8_t LED_PIN = 18;     // Define o pino do LED
const uint8_t BUTTON_PIN = 19;  // Define o pino do botão

Button btn(BUTTON_PIN);         // Cria um objeto Button para o botão como input pull-up

bool statusLED = LOW;           // Status do LED  (LOW = Desligado / HIGH = Ligado)

// Credenciais de rede WiFi
const char* ssid     = SSID;
const char* password = PASSWORD;

// Criação do servidor web e WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi Conectado, IP:");
  Serial.println(WiFi.localIP());
}

// Evento WiFi Desconectado
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Desconectado do WiFi");
  Serial.print("WiFi perdeu a conexão. Razão: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Tentando reconectar");
  WiFi.begin(ssid, password);
}

// Função para notificar todos os clientes conectados com o estado atual
void notifyClients() {
    ws.textAll(statusLED ? "on" : "off");
}

// Função para definir o estado do LED e notificar os clientes
void setLED(bool state) {
  statusLED = state;
  digitalWrite(LED_PIN, statusLED ? HIGH : LOW);
  notifyClients();
}

// Função para lidar com o pressionamento do botão
void handleButtonPress() {
  statusLED = !statusLED;
  setLED(statusLED);
  Serial.println("Botão pressionado, LED " + String(statusLED ? "ligado" : "desligado"));
}

// Função para lidar com eventos do WebSocket
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%lu connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      client->text(String(statusLED ? "on" : "off"));     
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA: {
      String msg = "";
      for (size_t i = 0; i < len; i++) {
        msg += (char)data[i];
      }
      Serial.println("Mensagem recebida via WebSocket: " + msg);
      if (msg == "on") {
        setLED(true);
      } else if (msg == "off") {
        setLED(false);
      }
      break;
    }
    default:
      break;
  }
}

void handleRoot(AsyncWebServerRequest *request){
  Serial.println("Acessando a página principal");
  request->send(200, "text/html", index_html);    // Retorna o HTML para o cliente
}

void handleNotFound(AsyncWebServerRequest *request) {
  Serial.println("Página não encontrada");
  request->send(404, "text/plain", "Not found");  // Retorna uma resposta 404 Not Found
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);          // Configura o pino do LED como saída
  digitalWrite(LED_PIN, LOW);        // Garante que o LED comece desligado

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);

  WiFi.disconnect(true);  // Desconecta de qualquer rede WiFi
  delay(1000);            // Aguarda 1 segundo

  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  //Conecta na rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ws.onEvent(onWebSocketEvent); // Registra o evento do WebSocket
  server.addHandler(&ws);       // Adiciona o WebSocket ao servidor   

  // Definicao de Rotas do Servidor (como um Backend)
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  server.begin(); // Inicia o servidor web
  Serial.println("Servidor iniciado");
}

void loop() {
  btn.update();         // Atualiza o estado do botão
  if (btn.wasPressed()) {
    handleButtonPress(); // Chama a função para lidar com o pressionamento do botão
  }
  // Limpa conexões WebSocket desconectadas
  ws.cleanupClients();
  delay(10); // Pequena pausa para evitar sobrecarga do loop
}
