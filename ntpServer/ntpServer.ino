#include <WiFi.h>         // Inclui a biblioteca WiFi para conectar ao WiFi
#include "credentials.h"  // Inclui o arquivo de credenciais do WiFi
#include <time.h>         // Inclui a biblioteca time para manipulação de tempo

// Configuração do NTP Server
const char* ntpServer  = "pool.ntp.org";  // Servidor NTP para sincronização de tempo  pool.ntp.org
const int32_t gmtOffset_sec  = -3 * 3600; // Offset de tempo em segundos (UTC -3 horas para o Brasil) 
const int32_t daylightOffset_sec  = 0;    // Offset de horário de verão (0 para desativado)

// Credenciais de rede WiFi
const char* ssid     = SSID;
const char* password = PASSWORD;

// Evento WiFi IP
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("WiFi Conectado, IP:");
  Serial.println(WiFi.localIP());
}

// Evento WiFi Desconectado
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.print("WiFi perdeu a conexão. Razão: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Tentando reconectar");
  WiFi.begin(ssid, password);
}

void printTime() {
  struct tm timeinfo; // Estrutura de tempo tm chamada timeinfo
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter tempo via NTP");
    return;
  }
  // Exibe a hora atual formatada com especificadores de formato
  Serial.println(&timeinfo, "Tempo atual: %H:%M:%S - %d/%m/%Y");
  Serial.println(&timeinfo, "Tempo atual: %A, %B %d %Y %H:%M:%S");
  Serial.println(&timeinfo, "Tempo atual: %c");

  Serial.println(&timeinfo, "Segundos: %S");

  char timeWeekDay[10];                                     // Variável para armazenar o dia da semana
  strftime(timeWeekDay,10, "%A", &timeinfo);                // Formata o dia da semana
  Serial.println("Dia da semana: " + String(timeWeekDay));  // Exibe o dia da semana formatado
}

void setup() {
  Serial.begin(115200);

  // Configuração WiFi
  WiFi.mode(WIFI_STA);     // Define como cliente Wi-Fi
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.disconnect(true);  // Desconecta de qualquer rede WiFi
  delay(1000);            // Aguarda 1 segundo

  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Conecta na rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Configuração do NTP Server e sincronização de tempo
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Aguarda até conseguir pegar o tempo
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Aguardando tempo via NTP...");
    delay(2000);
  }
  // Desconecta do WiFi após obter o horário
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop() {
  printTime();  // Exibe a hora atual
  delay(250);   // Aguarda 250 milissegundos
}
