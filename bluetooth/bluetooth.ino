#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Definição dos pinos
// #define MOTOR_LEFT_IN1 12   // Motor 1 - Frente
// #define MOTOR_LEFT_IN2 11   // Motor 1 - Trás
// #define MOTOR_RIGHT_IN3 10  // Motor 2 - Frente
// #define MOTOR_RIGHT_IN4 9   // Motor 2 - Trás
// #define ENA A5              // Controle de velocidade Motor 1
// #define ENB A6              // Controle de velocidade Motor 2

#define LED 2

// UUIDs do serviço UART BLE
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define DEVICE_INFO_SERVICE_UUID BLEUUID((uint16_t)0x180A)
#define MANUFACTURER_NAME_UUID BLEUUID((uint16_t)0x2A29)
#define FIRMWARE_REVISION_UUID BLEUUID((uint16_t)0x2A26)
#define MODEL_NUMBER_UUID BLEUUID((uint16_t)0x2A24)

// Ponteiro para o servidor e característica
BLEServer* pServer = nullptr;
BLECharacteristic* pTxCharacteristic = nullptr;

// Variáveis de controle de conexão
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Prototipo de funcoes
void stopMotors();
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void setSpeed(int speedMotor);

// Callback de conexão
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Cliente conectado.");
  }

  // Callback de desconexão
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Cliente desconectado.");
  }
};

// Callback para escrita
// Esta classe é chamada quando o cliente BLE escreve um valor na característica
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    String value = pCharacteristic->getValue();  // Obtém o valor escrito pelo cliente
    if (value.length() > 0) {
      Serial.println("Recebido do smartphone:");
      Serial.println(value);
      // (Opcional) ecoa o texto de volta ao app
      pTxCharacteristic->setValue(value);
      pTxCharacteristic->notify();

      char command = value[0];
      Serial.println(command);
      
      switch (command) {
        case 'F': moveForward(); break;
        case 'B': moveBackward(); break;
        case 'L': turnLeft(); break;
        case 'R': turnRight(); break;
        default: stopMotors(); break;
      }
    }
  }
};

// --- Funções de controle do motor ---
void stopMotors() {
  //   digitalWrite(MOTOR_LEFT_IN1, LOW);
  //   digitalWrite(MOTOR_LEFT_IN2, LOW);
  //   digitalWrite(MOTOR_RIGHT_IN3, LOW);
  //   digitalWrite(MOTOR_RIGHT_IN4, LOW);
  //   setSpeed(0);
  digitalWrite(LED, LOW);
  rgbLedWrite(LED, 0, 0, 0);  // Off / black
}

void moveForward() {
  //   digitalWrite(MOTOR_LEFT_IN1, HIGH);
  //   digitalWrite(MOTOR_LEFT_IN2, LOW);
  //   digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  //   digitalWrite(MOTOR_RIGHT_IN4, LOW);
  //   setSpeed(speed);
  rgbLedWrite(LED, 0, 0, 64);  // Blue
}

void moveBackward() {
  //   digitalWrite(MOTOR_LEFT_IN1, LOW);
  //   digitalWrite(MOTOR_LEFT_IN2, HIGH);
  //   digitalWrite(MOTOR_RIGHT_IN3, LOW);
  //   digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  //   setSpeed(speed);
  rgbLedWrite(LED, 64, 0, 0);  // Red
}

void turnLeft() {
  // Gira no próprio eixo: uma roda para frente, outra para trás
  //   digitalWrite(MOTOR_LEFT_IN1, LOW);
  //   digitalWrite(MOTOR_LEFT_IN2, HIGH);
  //   digitalWrite(MOTOR_RIGHT_IN3, HIGH);
  //   digitalWrite(MOTOR_RIGHT_IN4, LOW);
  //   setSpeed(speed);
  rgbLedWrite(LED, 0, 64, 0);  // Green
}

void turnRight() {
  // Gira no próprio eixo: uma roda para frente, outra para trás
  //   digitalWrite(MOTOR_LEFT_IN1, HIGH);
  //   digitalWrite(MOTOR_LEFT_IN2, LOW);
  //   digitalWrite(MOTOR_RIGHT_IN3, LOW);
  //   digitalWrite(MOTOR_RIGHT_IN4, HIGH);
  //   setSpeed(speed);
  digitalWrite(LED, HIGH);
}

void setSpeed(int speedMotor) {
  //   analogWrite(ENA, speedMotor);  // Define a velocidade do motor 1
  //   analogWrite(ENB, speedMotor);  // Define a velocidade do motor 2
}

void setup() {
  Serial.begin(115200);

  //   pinMode(MOTOR_LEFT_PIN1, OUTPUT);
  //   pinMode(MOTOR_LEFT_PIN2, OUTPUT);
  //   pinMode(MOTOR_RIGHT_PIN1, OUTPUT);
  //   pinMode(MOTOR_RIGHT_PIN2, OUTPUT);
  //   pinMode(ENA, OUTPUT);
  //   pinMode(ENB, OUTPUT);
  pinMode(LED, OUTPUT);

  // Inicialmente para todos os motores desligados
  stopMotors();

  // Inicializa o BLE
  BLEDevice::init("ESP32C3_CAR");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Cria o serviço BLE
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Característica TX (ESP32 → Smartphone)
  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  // Característica RX (Smartphone → ESP32)
  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Inicia o serviço
  pService->start();
  pServer->getAdvertising()->start();

  // // Configuração do Serviço de Informações do Dispositivo
  // BLEService* deviceInfoService = pServer->createService(DEVICE_INFO_SERVICE_UUID);

  // deviceInfoService->createCharacteristic(MANUFACTURER_NAME_UUID, BLECharacteristic::PROPERTY_READ)
  //   ->setValue("Guilherme");

  // deviceInfoService->createCharacteristic(FIRMWARE_REVISION_UUID, BLECharacteristic::PROPERTY_READ)
  //   ->setValue("v1.0");

  // deviceInfoService->createCharacteristic(MODEL_NUMBER_UUID, BLECharacteristic::PROPERTY_READ)
  //   ->setValue("ESP32C3 mini");

  // deviceInfoService->start();

  // // Advertising
  // BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  // pAdvertising->setScanResponse(true);
  // pAdvertising->addServiceUUID(DEVICE_INFO_SERVICE_UUID);
  // pAdvertising->start();

  Serial.println("BLE pronto.");
}

void loop() {
  // Enviar texto do monitor serial para o smartphone(ESP32 → Smartphone)
  //   static String inputSerial = "";

  //   while (Serial.available()) {
  //     char c = Serial.read();
  //     if (c == '\n' || c == '\r') {
  //       if (inputSerial.length() > 0 && deviceConnected) {
  //         Serial.print("Enviando para smartphone: ");
  //         Serial.println(inputSerial);

  //         pTxCharacteristic->setValue(inputSerial);
  //         pTxCharacteristic->notify();
  //         inputSerial = "";
  //       }
  //     } else {
  //       inputSerial += c;
  //     }
  //   }

  // Reconectar advertising após desconexão
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Reiniciando advertising...");
    oldDeviceConnected = deviceConnected;
  }

  // Novo cliente conectado
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
    Serial.println("Novo cliente conectado.");
  }
}
