/*
    * ESP32C6 - UART Receiver 
    * Baud rate: 9600 bps
    * start bit + data bit (8) + stop bit
*/

#define BIT_BANGING  // Altera entre codigo uart bit banging e usando serial

#define TX_PIN 16
#define RX_PIN 17
#define BAUD_RATE 9600
#define BIT_DURATION (1000000 / BAUD_RATE)  // Duração do bit em microssegundos. (1 / 9600s) * 1.000.000 us/s ~= 104us

const int MAX_MSG = 64;
char buffer[MAX_MSG + 1];  // garante espaço para \0
int buf_idx = 0;
int MAX_WAIT_TIME = 10;  // tempo de aguardo para saber que encerrou

void setup() {
  Serial.begin(BAUD_RATE);
#ifdef BIT_BANGING
  pinMode(RX_PIN, INPUT);
#else
  Serial1.begin(BAUD_RATE, SERIAL_8N1, RX_PIN, TX_PIN);
#endif
}

void loop() {
#ifdef BIT_BANGING
  static unsigned long startTime = 0;
  // Verifica se um start bit (transição de HIGH para LOW) foi detectado
  if (digitalRead(RX_PIN) == LOW) {
    char dado = uartReceiveByte();
    // armazena no buffer até max ou tempo de espera encerrar
    if (buf_idx < MAX_MSG) {
      buffer[buf_idx++] = dado;
      startTime = millis();
    }
  }
  if ((millis() - startTime > MAX_WAIT_TIME) || buf_idx == MAX_MSG) {
    if (buf_idx > 0) {
      buffer[buf_idx] = '\0';  // termino da string
      Serial.print("Recebido: ");
      Serial.println(buffer);
      buf_idx = 0;  // reinicia
    }
  }
#else
  if (Serial1.available()) {
    String str = Serial1.readStringUntil('\n');
    Serial.print("Recebido da Franzininho: ");
    Serial.println(str);
  }
  if (Serial.available()) {
    char a = Serial.read();  // Lê aaractere da USB
    Serial.print("Recebido do Monitor Serial: ");
    Serial.println(a);
    Serial1.write(a);  // Envia para a Franzininho
  }
#endif
}

char uartReceiveByte() {
  delayMicroseconds(BIT_DURATION + BIT_DURATION / 2);  // move para meio do primeiro bit de dados
  uint8_t dado = 0;
  for (int i = 0; i < 8; i++) {
    dado |= (digitalRead(RX_PIN) << i);
    delayMicroseconds(BIT_DURATION);
  }
  // stop bit
  delayMicroseconds(BIT_DURATION);
  return (char)dado;
}