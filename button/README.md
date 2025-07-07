# Biblioteca Button para Arduino

## 📌 Visão Geral

Esta biblioteca fornece uma interface simples e eficiente para o uso de botões físicos com o ESP32-C6, incluindo funcionalidades como debounce, detecção de press/release e eventos.

## 🔧 Funcionalidades

- Debounce por software de 50ms
- Detecção de:
  - Pressionado (wasPressed)
  - Solta (wasReleased)
  - Estado atual (isPressed)
- Suporte a múltiplos botões
- Configuração com ou sem pull-up interno

## 📦 Instalação

1. Copie a pasta `button/` para o diretório `Documentos/Arduino/libraries/`
2. Reinicie a Arduino IDE
3. Inclua a biblioteca no seu sketch com: #include <Button.h>

## 📝 Exemplo de uso

```cpp
#include <Button.h>

const uint8_t BOTAO_1 = 7; // Pino do Botão 1

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
```

## 🚀 Funções Públicas

### `void update()`

Atualiza o estado do botão (deve ser chamada em loop)

### `bool wasPressed()`

Retorna true se o botão foi pressionado (bordo de descida)

### `bool isPressed()`

Retorna true se o botão está pressionado

### `bool wasReleased()`

Retorna true se o botão foi solto (bordo de subida)

---

## 📜 Licença

Este projeto é licenciado sob a [MIT License](LICENSE) - consulte o arquivo `LICENSE` para mais detalhes.

## 💻 Autor

Desenvolvido por [Guilherme Fernandes](https://github.com/guilhermefernandesk)

<a href="https://www.linkedin.com/in/iguilherme" target="_blank"><img src="https://img.shields.io/badge/-LinkedIn-%230077B5?style=for-the-badge&logo=linkedin&logoColor=white" target="_blank"></a>

---
