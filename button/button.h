#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
    public:
        // Construtor: define o pino e o modo de entrada (padrão: INPUT_PULLUP)
        Button(uint8_t pin, bool inputPullup = true);

        void update();          // Atualiza o estado do botão (deve ser chamada no loop)
        bool wasPressed();      // Retorna true se o botão foi pressionado (bordo de descida)
        bool isPressed();       // Retorna true se o botão está pressionado
        bool wasReleased();     // Retorna true se o botão foi solto (bordo de subida)

    private:
        uint8_t _pin;                    // Pino físico do botão
        bool _inputPullup;               // true se usa INPUT_PULLUP
        bool _state;                     // Estado atual (HIGH/LOW)
        bool _lastState;                 // Estado anterior
        bool _pressedEvent;              // Flag de evento de pressionado
        bool _releasedEvent;             // Flag de evento de solto
        unsigned long _lastDebounceTime; // Timestamp do último bounce
        unsigned long _debounceDelay;    // Tempo de debounce em ms
};

#endif