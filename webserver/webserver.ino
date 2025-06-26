#include <WiFi.h>       // Inclui a biblioteca WiFi para conectar ao WiFi
#include <WebServer.h>  // Inclui a biblioteca WebServer para criar um servidor web
#include "html_page.h"  // Inclui o arquivo HTML que contém a página web

const uint8_t LED_PIN = 18;     // Define o pino do LED
const uint8_t BUTTON_PIN = 19;  // Define o pino do botão

bool statusLED = LOW;           // Status do LED  (LOW = Desligado / HIGH = Ligado)

// Credenciais de rede WiFi
const char* ssid     = "****";
const char* password = "****";

WebServer server(80);

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

void handleStatus() {
  server.send(200, "text/plain", statusLED ? "on" : "off"); // Retorna o status do LED como texto simples
  Serial.println("Status do LED: ");
  Serial.println(statusLED ? "on" : "off");                 //Imprime o status do LED
}

void LEDon(){
  statusLED = HIGH;
  digitalWrite(LED_PIN, HIGH);
  server.send(200, "text/plain", "on");  // Retorna "on" como status do LED
  Serial.println("LED ligado");          // Imprime no console que o LED foi ligado
}

void LEDoff(){
  statusLED = LOW;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "off"); // Retorna "off" como status do LED
  Serial.println("LED desligado");       // Imprime no console que o LED foi desligado
}

void handleRoot(){
  Serial.println("Acessando a página principal");
  server.send(200, "text/html", index_html); // Retorna o HTML para o cliente
}

void handleNotFound() {
  Serial.println("Página não encontrada");
  server.send(404, "text/plain", "Not found"); // Retorna uma resposta 404 Not Found
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);          // Configura o pino do LED como saída
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Configura o pino do botão como entrada com pull-up interno
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

  // Definicao de Rotas do Servidor (como um Backend)
  server.on("/", handleRoot);
  server.on("/on", LEDon);
  server.on("/off", LEDoff);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);

  server.begin(); // Inicia o servidor web
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();

  // Verifica o estado do botão
  if (digitalRead(BUTTON_PIN) == LOW) {             // Botão pressionado
    while (digitalRead(BUTTON_PIN) == LOW);         // Aguarda o botão ser solto
    statusLED = !statusLED;                         // Inverte o estado do LED
    digitalWrite(LED_PIN, statusLED ? HIGH : LOW);  // Atualiza o LED
    Serial.println("Botão pressionado, LED atualizado");
  }

  delay(10); // Pequena pausa para evitar sobrecarga do loop
}
