/*
    * Franzinho WiFi - UART
    * Baud rate: 9600 bps
    * start bit + data bit (8) + stop bit
*/

#define BIT_BANGING  // Altera entre codigo uart bit banging e usando serial

#define TX_PIN 17
#define RX_PIN 18
#define BAUD_RATE 9600
#define BIT_DURATION (1000000 / BAUD_RATE)  // Duração do bit (1 / 9600s) * 1.000.000 us/s ~= 104us

const uint8_t BUTTON_PIN = 0;  // Pino do botão boot da franzininho

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#ifdef BIT_BANGING
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, HIGH);  // linha ociosa em HIGH
#else
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
#endif
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
    if ((endTime - startTime) > 10000) {  // botao pressionado por 10 segundos
#ifdef BIT_BANGING
      uartSendString("Hello World");  // Envia string
#else
      Serial1.println("Hello World");  // Envia string
#endif
    } else if ((endTime - startTime) > 3000) {  // botao pressionado por 3 segundos
#ifdef BIT_BANGING
      uartSendByte(0x41);  // Envia caractere 'A' (0x41) - (0b01000001)
#else
      Serial1.write(0x41);             // Envia caractere 'A' (0x41) - (0b01000001)
#endif
    }
  }
#ifndef BIT_BANGING
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    Serial.print("Do Monitor Serial: ");
    Serial.println(input);
    Serial1.println(input);
  }
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.print("Recebido: ");
    Serial.println(c);
  }
#endif
}

void uartSendByte(uint8_t bits) {
  // Start bit
  digitalWrite(TX_PIN, LOW);
  delayMicroseconds(BIT_DURATION);

  // Envia 8 bits (LSB primeiro)
  for (int i = 0; i < 8; i++) {
    digitalWrite(TX_PIN, (bits >> i) & 0x01);
    delayMicroseconds(BIT_DURATION);
  }
  // Stop bit
  digitalWrite(TX_PIN, HIGH);
  delayMicroseconds(BIT_DURATION);
}

void uartSendString(const char* str) {
  while (*str != '\0') {
    uartSendByte(*str++);  // recebe vetor de char e aponta para cada caracter
    delayMicroseconds(BIT_DURATION);
  }
}