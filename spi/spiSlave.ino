/*
    * ESP32C6-DevkitC-1 - Slave SPI
    * CPHA = 0, CPOL = 0
    * SPI Mode 0
    * Frequência de clock: 100kHz
    * https://en.wikipedia.org/wiki/Serial_Peripheral_Interface
*/

#define SS 18    // Slave Select
#define MOSI 19  // Master Out Slave In
#define MISO 20  // Master In Slave Out
#define SCK 21   // Serial Clock

const uint8_t DATA_SEND = 0x25;   // Valor para enviar ao mestre
const uint8_t DATA_EXPECT = 0x19; // Valor esperado do mestre para acender o led

void setup() {
  Serial.begin(115200);
  pinMode(MOSI, INPUT);   // Entrada para receber a saida do mestre
  pinMode(SCK, INPUT);    // Clock do mestre
  pinMode(SS, INPUT);     // Slave Select saida do mestre
  pinMode(MISO, OUTPUT);  // Saida  do escravo para entrada no mestre

  digitalWrite(MISO, LOW);
}

void loop() {
  if (digitalRead(SS) == LOW) { // Escravo selecionado
    uint8_t recebido = 0;
    // Recebe bits 7..0
    for (int i = 7; i >= 0; i--) {
      // Espera subir o clock
      while (digitalRead(SCK) == LOW);

      // Le bit enviado pelo mestre
      recebido |= ((digitalRead(MOSI)) << i);

      // Define bit para resposta
      digitalWrite(MISO, (DATA_SEND >> i) & 0x01);

      // Espera descer o clock
      while (digitalRead(SCK) == HIGH);
    }

    Serial.print("Recebido do mestre: 0x");
    Serial.println(recebido, HEX);

    if (recebido == DATA_EXPECT) {
      Serial.println("Comunicação bem sucedida!");
      rgbLedWrite(RGB_BUILTIN, 0, 0, 64);  // Blue
      delay(1000);
      rgbLedWrite(RGB_BUILTIN, 0, 0, 0);  // Off / black
    }
  }
}
