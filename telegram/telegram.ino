#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOTtoken "xxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"  // Seu Telegram Bot Token (Obtenha do BotFather)
#define CHAT_ID "xxxxxxxxxx"                                       // Use @myidbot para obter o ID do chat

const uint8_t LED_PIN = 18;

// Configuração WiFi
const char* ssid = "xxxxxxxxx";
const char* password = "xxxxxxxxxx";

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int botRequestDelay = 1000;  // Intervalo entre requisições ao bot de 1 segundo
unsigned long lastTimeBot;         // Armazena o último tempo em que o bot foi executado

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Certificado raiz para api.telegram.org
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBot + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBot = millis();
  }
}

void handleNewMessages(int numNewMessages) {
  Serial.println(String(numNewMessages));
  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Usuário não autorizado", "");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println("Mensagem recebida: " + text);
    if (text == "/start") {
      String welcome = "Bem-vindo, " + String(bot.messages[i].from_name) + ".\n";
      welcome += "Use os seguintes comandos para controlar suas saídas.\n\n";
      welcome += "/led_on para ligar o GPIO \n";
      welcome += "/led_off para desligar o GPIO \n";
      welcome += "/state para solicitar o estado atual do GPIO \n";
      bot.sendMessage(chat_id, welcome, "");
    } else if (text == "/led_on") {
      digitalWrite(LED_PIN, HIGH);
      bot.sendMessage(chat_id, "LED ON", "");
    } else if (text == "/led_off") {
      digitalWrite(LED_PIN, LOW);
      bot.sendMessage(chat_id, "LED OFF", "");
    } else if (text == "/state") {
      if (digitalRead(LED_PIN)) {
        bot.sendMessage(chat_id, "LED ON", "");
      } else {
        bot.sendMessage(chat_id, "LED OFF", "");
      }
    } else {
      bot.sendMessage(chat_id, "Comando inválido. Use /start para ajuda.", "");
    }
  }
}