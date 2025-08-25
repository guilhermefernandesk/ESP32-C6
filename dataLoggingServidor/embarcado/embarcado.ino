#include <WiFi.h>       // Inclui a biblioteca WiFi para conectar-se a redes sem fio
#include <HTTPClient.h> // Inclui a biblioteca HTTPClient para fazer requisições HTTP
#include "DHT.h"        // Inclui a biblioteca DHT para sensores de temperatura e umidade
#include "credentials.h"

#define DHTTYPE DHT11       // Tipo de sensor DHT11
const uint8_t DHTPIN = 18;  // Pino onde o sensor DHT está conectado
DHT dht(DHTPIN, DHTTYPE);   // Inicializa o sensor DHT

// Credenciais de rede WiFi
const char* ssid = SSID;
const char* password = PASSWORD;
String serverPath = API_URLs;   // "http://<IP_DO_SERVIDOR>:<PORTA>"

// Identificadores do dispositivo e sensor
String deviceID = "esp32c6-01";
String sensorName = "DHT11";
String sensorLocation = "Casa";

// Variáveis para armazenar leituras de sensores
float temperature, humidity;

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

void setup() {
  Serial.begin(115200);

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
  Serial.println("\nConectado");

  // Inicializa o sensor DHT
  dht.begin();
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Verifica se a leitura do sensor DHT falhou.
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Falha ao ler o sensor DHT!");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http; // Cria uma instância do cliente HTTP
    http.begin((serverPath + "/sensor").c_str()); // Inicia a conexão com o servidor na rota
    http.addHeader("Content-Type", "application/json"); // Adiciona o cabeçalho Content-Type

    // Cria o payload JSON
    String payload = String(
                      "{\"device_id\":\"")
                    + deviceID 
                    + "\",\"sensor\":\"" + sensorName 
                    + "\",\"location\":\"" + sensorLocation 
                    + "\",\"value1\":\"" + temperature 
                    + "\",\"value2\":\"" + humidity + "\"}";

    int code = http.POST(payload);
    if (code != 201) {
      Serial.printf("Erro ao enviar dados: %s\n", http.errorToString(code).c_str());
    }

    Serial.println("Dados enviados com sucesso");
    String resp = http.getString();
    Serial.println(resp);
    http.end();
  }
  delay(5000);  // Aguarda 5 segundos antes de enviar novos dados
}