/*
    Placa Franzininho WiFi Lab01
*/

#include "button.h"

//Definições de perifericos externos da placa
// LEDs
const uint8_t LED_VERDE    = 13; // Pino do LED Verde
// Botões
const uint8_t BOTAO_1 = 7;       // Pino do Botão 1


// Construtor: define o pino e o modo de entrada (padrão: INPUT_PULLUP)
Button btn(BOTAO_1); // Pino conectado ao botão

void setup() {
  Serial.begin(115200);
}

void loop() {
  btn.update();
  if (btn.wasPressed()) {
    Serial.println("Botão pressionado");
  }
  if (btn.wasReleased()) {
    Serial.println("Botão solto");
  }
}


