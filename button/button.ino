#include "button.h"

//Definições de perifericos externos da placa
// LEDs
const uint8_t LED_VERMELHO = 14; // Pino do LED Vermelho
const uint8_t LED_VERDE    = 13; // Pino do LED Verde
const uint8_t LED_AZUL     = 12; // Pino do LED Azul

// Botões
const uint8_t BOTAO_1 = 7;       // Pino do Botão 1
const uint8_t BOTAO_2 = 6;       // Pino do Botão 2
const uint8_t BOTAO_3 = 5;       // Pino do Botão 3
const uint8_t BOTAO_4 = 4;       // Pino do Botão 4
const uint8_t BOTAO_5 = 3;       // Pino do Botão 5
const uint8_t BOTAO_6 = 2;       // Pino do Botão 6

// Buzzer
const uint8_t BUZZER = 17; // Pino do Buzzer

// Array de LEDs
const uint8_t leds[] = { 
  LED_VERMELHO,
  LED_VERDE,
  LED_AZUL
};

// Array de Botões
Button btns[] = {
  Button(BOTAO_1),
  Button(BOTAO_2),
  Button(BOTAO_3),
  Button(BOTAO_4),
  Button(BOTAO_5),
  Button(BOTAO_6)
};

// Enum para os botoes
enum ID_BOTAO {
  ID_BOTAO_1,
  ID_BOTAO_2,
  ID_BOTAO_3,
  ID_BOTAO_4,
  ID_BOTAO_5,
  ID_BOTAO_6
};

// Definição do número de botões e LEDs
const int numBotoes = sizeof(btns) / sizeof(btns[0]);
const int numLeds = sizeof(leds) / sizeof(leds[0]);

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < numLeds; i++) {
    pinMode(leds[i], OUTPUT);      // Configura os pinos dos LEDs como saída
    digitalWrite(leds[i], LOW);    // Inicializa os LEDs como desligados
  }
  pinMode(BUZZER, OUTPUT);         // Configura o Buzzer como saída
  digitalWrite(BUZZER, LOW);       // Inicializa o Buzzer como desligado
  
}

void loop() {
  for (int i = 0; i < numBotoes; i++) {
    btns[i].update();         // Atualiza o estado de cada botão
  
    if (btns[i].wasPressed()){  // Verifica se algum botão foi pressionado
      switch (i) {              // Identifica qual botão foi pressionado
        case ID_BOTAO_1:
          Serial.println("Botão 1 pressionado");
          digitalWrite(LED_VERMELHO, !digitalRead(LED_VERMELHO)); // Alterna o LED Vermelho
          break;
        case ID_BOTAO_2:
          Serial.println("Botão 2 pressionado");
          digitalWrite(LED_VERDE, !digitalRead(LED_VERDE));       // Alterna o LED Verde
          break;
        case ID_BOTAO_3:
          Serial.println("Botão 3 pressionado");
          digitalWrite(LED_AZUL, !digitalRead(LED_AZUL));         // Alterna o LED Azul
          break;
        case ID_BOTAO_4:
          Serial.println("Botão 4 pressionado");
          break;
        case ID_BOTAO_5:
          Serial.println("Botão 5 pressionado");
          // Ação para o Botão 5
          break;
        case ID_BOTAO_6:
          Serial.println("Botão 6 pressionado");
          // Ação para o Botão 6
          break;
      }
    }
  }
  delay(100); // Pequeno atraso para evitar leituras muito rápidas
}
