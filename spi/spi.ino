/*
    * Franzinho WiFi - Master SPI
    * CPHA = 0, CPOL = 0
    * SPI Mode 0
    * Frequência de clock: 100kHz
    * https://en.wikipedia.org/wiki/Serial_Peripheral_Interface
*/

#define SS 10    // Slave Select
#define MOSI 11  // Master Out Slave In
#define MISO 12  // Master In Slave Out
#define SCK 13   // Serial Clock

#define SPI_FREQ_US 5  // 5µs

const uint8_t BUTTON_PIN = 0;      // Pino do botão
const uint8_t DATA_SEND = 0x19;    // Valor para acender led no escravo
const uint8_t DATA_EXPECT = 0x25;  // Valor esperado do retorno do escravo

// Funcao para inicializar o SPI
void spiInit() {
  pinMode(MOSI, OUTPUT);  // MOSI é saída no mestre
  pinMode(MISO, INPUT);   // MISO é entrada no mestre
  pinMode(SCK, OUTPUT);   // SCK é saída no mestre
  pinMode(SS, OUTPUT);    // SS é saída no mestre

  digitalWrite(SCK, LOW);  // Inicializa o clock em LOW
  digitalWrite(SS, HIGH);  // Desativa o escravo SPI
  digitalWrite(MOSI, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

// Funcao para exibir os pinos SPI para defini-los
void spiPinout() {
  delay(3000);
  Serial.print("MOSI: ");
  Serial.println(MOSI);
  Serial.print("MISO: ");
  Serial.println(MISO);
  Serial.print("SCK: ");
  Serial.println(SCK);
  Serial.print("SS: ");
  Serial.println(SS);
}

// Funcao controle de clock 100kHz
void spiPulseClock() {
  // 1 ciclo = ( 1 / 100 kHz = 10 µs)
  // HIGH = 5 µs e LOW = 5 µs
  digitalWrite(SCK, HIGH);
  delayMicroseconds(SPI_FREQ_US);
  digitalWrite(SCK, LOW);
  delayMicroseconds(SPI_FREQ_US);
}

uint8_t spiSendByte(uint8_t data) {
  uint8_t data_in = 0;             // receber do escravo
  digitalWrite(SS, LOW);           // seleciona slave e inicia a comunicacao
  delayMicroseconds(SPI_FREQ_US);  // garante a janela de comunicacao
  // envia bits 7..0
  for (int i = 7; i >= 0; i--) {
    // Coloca bit a bit na linha MOSI
    digitalWrite(MOSI, (data >> i) & 0x01);
    // Pulso de clock
    spiPulseClock();
    // Lê bit do escravo no MISO
    data_in |= ((digitalRead(MISO)) << i);
  }
  digitalWrite(SS, HIGH);  // Desativa slave e finaliza comunicação
  return data_in; // dado recebido
}

void setup() {
  Serial.begin(115200);
  spiPinout();
  spiInit();
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) { 
    // debounce
    delay(100);
    int startTime = millis();
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
    }
    int endTime = millis();
    if ((endTime - startTime) > 3000) {  //botao pressionado por 3 segundos
      Serial.println("Botão pressionado. Enviando comando 0x19...");
      // Envia byte para o escravo
      uint8_t resposta = spiSendByte(DATA_SEND);
      Serial.print("Resposta do escravo: 0x");
      Serial.println(resposta, HEX);
      if (resposta == DATA_EXPECT) { // Espera receber o valor 0x25 do escravo
        Serial.println("Comunicação bem sucedida!");
      }
    }
  }
}