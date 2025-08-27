#include <WiFi.h>         // Inclui a biblioteca WiFi para conectar ao WiFi
#include <WebServer.h>    // Inclui a biblioteca WebServer para criar um servidor web
#include <ArduinoJson.h>  // Inclui a biblioteca ArduinoJson para manipulação de JSON
#include "DHT.h"          // Inclui a biblioteca DHT para sensores de temperatura e umidade
#include "credentials.h"

#define DHTTYPE DHT11       // Tipo de sensor DHT11
const uint8_t DHTPIN = 19;  // Pino onde o sensor DHT está conectado
DHT dht(DHTPIN, DHTTYPE);

const uint8_t LED_PIN = 18;  // Define o pino do LED

bool statusLED = LOW;  // Status do LED  (LOW = Desligado / HIGH = Ligado)

// Variáveis para armazenar leituras de sensores
float temperature, humidity;

// Credenciais de rede WiFi
const char* ssid = SSID;
const char* password = PASSWORD;

WebServer server(80);  // Cria uma instância do servidor web na porta 80

// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
  Serial.println("WiFi Conectado, IP:");
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

// Função para definir o estado do LED
void setLED(bool state) {
  statusLED = state;
  digitalWrite(LED_PIN, statusLED);
}

// Função para ler os sensores
bool readSensor() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Verifica se a leitura do sensor DHT falhou.
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler o sensor DHT!");
    return false;
  }
  return true;
}

// Função para lidar com requisições de sensor
void handleSensor() {
  if (readSensor()) {
    JsonDocument doc;
    doc["temperatura"] = temperature;
    doc["umidade"] = humidity;
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);  // Envia a resposta com os dados do sensor
  } else {
    server.send(500, "application/json", "{\"error\":\"Falha no sensor\"}");
  }
}

// Função para lidar com requisições de LED
void handleLed() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");  // Obtém o corpo da requisição POST

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (!error) {
      setLED(doc["led"]);
      server.send(200, "application/json", "{\"message\":\"LED atualizado\"}");
      return;
    }
  }
  server.send(400, "application/json", "{\"error\":\"JSON inválido\"}");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);  // Configura o pino do LED como saída

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
  Serial.println("\nConectado!");

  // Inicializa o sensor DHT
  dht.begin();

  // Definicao de Rotas da API REST
  server.on("/sensor", HTTP_GET, handleSensor);
  server.on("/led", HTTP_POST, handleLed);

  server.begin();  // Inicia o servidor web server
  Serial.println("Servidor iniciado!");
}

void loop() {
  server.handleClient();
  delay(10);  // Pequena pausa para evitar sobrecarga do loop
}
