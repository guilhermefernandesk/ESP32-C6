/*********************************************************
 * Projeto: ESP32C6 Data Logging
 *
 * Descrição: Este projeto permite coletar dados de sensores
 *            e publicá-los em uma planilha do Google Sheets.
 *
 * Autor: Guilherme Fernandes
 * Data: 18/08/2025
 * Placa: ESP32-C6 DevKitC-1
 *********************************************************/

#include <WiFi.h>                     // Inclui a biblioteca WiFi para conectar ao WiFi
#include "time.h"                     // Inclui a biblioteca time para manipulação de tempo
#include <ESP_Google_Sheet_Client.h>  // Inclui a biblioteca para interagir com o Google Sheets
#include "DHT.h"                      // Inclui a biblioteca DHT para sensores de temperatura e umidade

#define DHTTYPE DHT11       // Tipo de sensor DHT11
const uint8_t DHTPIN = 18;  // Pino onde o sensor DHT está conectado

DHT dht(DHTPIN, DHTTYPE);

// Credenciais de rede WiFi
const char* SSID = "your_SSID";
const char* PASSWORD = "your_PASSWORD";

// Google Project ID     
const char* PROJECT_ID = "your_ProjectID";     
// Service Account's client email                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               // Google Project ID
const char* CLIENT_EMAIL = "your_ClientEmail";
// Service Account's private key   
const char* PRIVATE_KEY[] PROGMEM = "your_PrivateKey";
// The ID of the spreadsheet where you'll publish the data
const char* SPREADSHEET_ID = "your_SpreadsheetID";

const char* sheetName = "Sheet1";                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          // Google Sheet name

// Configuração do NTP Server
const char* ntpServer = "pool.ntp.org";   // Servidor NTP para sincronização de tempo  pool.ntp.org
const int32_t gmtOffset_sec = -3 * 3600;  // Offset de tempo em segundos (UTC -3 horas para o Brasil)
const int32_t daylightOffset_sec = 0;     // Offset de horário de verão (0 para desativado)

// Controle de tempo para envio de dados
unsigned long lastTime = 0;
unsigned long timerDelay = 10000; //10seconds

// Variáveis para armazenar leituras de sensores
float temp, hum;

// Estrutura para armazenar data e hora
struct DateTime {
  String date;  // dd/mm/yyyy
  String hour;  // HH:MM:SS
};

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
  WiFi.begin(SSID, PASSWORD);
}

// Função para obter a data e hora atuais
DateTime getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter tempo via NTP");
    return { "", "" };
  }
  char bufferDate[15];
  char bufferHour[10];

  strftime(bufferHour, sizeof(bufferHour), "%H:%M:%S", &timeinfo);
  strftime(bufferDate, sizeof(bufferDate), "%d/%m/%Y", &timeinfo);

  return { String(bufferDate), String(bufferHour) };
}

// Token Callback function
void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}

void setup() {
  Serial.begin(115200);

  // Configuração WiFi
  WiFi.mode(WIFI_STA);  // Define como cliente Wi-Fi
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.disconnect(true);  // Desconecta de qualquer rede WiFi
  delay(1000);            // Aguarda 1 segundo

  // Registra os eventos do WiFi
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Conecta na rede WiFi
  WiFi.begin(SSID, PASSWORD);
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

  // Inicializa o sensor DHT
  dht.begin();

  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

  // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

  // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop() {
  bool ready = GSheet.ready(); // Verifica se a planilha está pronta

  if (ready && millis() - lastTime > timerDelay) {
    lastTime = millis();

    FirebaseJson response;

    Serial.println("\nAppend spreadsheet values...");
    Serial.println("----------------------------");

    FirebaseJson valueRange;

    temp = dht.readTemperature();
    hum = dht.readHumidity();
    DateTime time = getTime();

    // Verifica se a leitura do sensor DHT falhou.
    if (isnan(hum) || isnan(temp)) {
      Serial.println("Falha ao ler o sensor DHT!");
      return;
    }

    // Verifica se o tempo foi obtido com sucesso.
    if (time.date == "" || time.hour == "") {
      Serial.println("Falha ao obter o tempo!");
      return;
    }

    // Os dados serão publicados na planilha em linhas
    valueRange.add("majorDimension", "ROWS");
    valueRange.set("values/[1]/[0]", time.date);
    valueRange.set("values/[1]/[1]", time.hour);
    valueRange.set("values/[1]/[2]", String(temp, 2));
    valueRange.set("values/[1]/[3]", String(hum, 2));

    // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
    // Append values to the spreadsheet
    bool success = GSheet.values.append(&response, SPREADSHEET_ID, String(sheetName) + "!A2", &valueRange);
    if (success) {
      response.toString(Serial, true);
      valueRange.clear();
    } else {
      Serial.println(GSheet.errorReason());
    }
    Serial.println();
    Serial.println(ESP.getFreeHeap());
  }
}
