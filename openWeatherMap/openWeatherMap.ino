#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "credentials.h"

// Credenciais de rede WiFi
const char* ssid = SSID;
const char* password = PASSWORD;

// OpenWeatherMap
String apiKey = API_KEY;
String cidade = "Sao%20Paulo,br";  // Espacos devem respeitar a URL-encode e substituir por %20
String url = "http://api.openweathermap.org/data/2.5/weather?q=" + cidade + "&appid=" + apiKey + "&units=metric&lang=pt";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.disconnect(true);  // Desconecta de qualquer rede WiFi
  delay(1000);            // Aguarda 1 segundo

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
}

void exampleCreateJson() {
  Serial.println("Funcao exampleCreateJson:");
  // Criar documento JSON
  JsonDocument doc;

  doc["temperatura"] = 25;
  doc["umidade"] = 65;
  doc["status"] = "OK";

  // Serializar para string
  String output;
  serializeJson(doc, output);

  // Enviar via Serial
  Serial.println(output);
}

void exampleReadJson() {
  Serial.println("Funcao exampleReadJson:");
  Serial.println("Envie um JSON no formato: {\"led\":1}");
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, input);

    if (!error) {
      int ledStatus = doc["led"];
      Serial.print("Valor recebido: ");
      Serial.println(ledStatus);

      if (ledStatus == 1) {
        Serial.println("LED ligado");
      } else {
        Serial.println("LED desligado");
      }
    } else {
      Serial.println("Erro ao interpretar JSON");
    }
  }
}

void getOpenWeatherMapData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  // Cria uma instância do cliente HTTP
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode == 200) {
      String payload = http.getString();
      // Criar documento JSON
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        float temperatura = doc["main"]["temp"];
        int umidade = doc["main"]["humidity"];
        const char* descricao = doc["weather"][0]["description"];
        const char* cidadeNome = doc["name"];

        Serial.println("------ Clima Atual ------");
        Serial.print("Cidade: ");
        Serial.println(cidadeNome);
        Serial.print("Temperatura: ");
        Serial.print(temperatura);
        Serial.println(" °C");
        Serial.print("Umidade: ");
        Serial.print(umidade);
        Serial.println(" %");
        Serial.print("Condição: ");
        Serial.println(descricao);
        Serial.println("-------------------------\n");
      } else {
        Serial.println("Erro ao interpretar JSON");
      }
    } else {
      Serial.print("Erro na requisição: ");
      Serial.println(httpCode);
    }
    http.end();
  }
}

void loop() {
  exampleCreateJson();      // Função exemplo de criação de JSON
  exampleReadJson();        // Função exemplo de leitura de JSON
  getOpenWeatherMapData();  // Função para obter dados do OpenWeatherMap
  delay(10000);             // Atualiza a cada 10s
}
