#include "button.h"

// Construtor com lista de inicialização
Button::Button(uint8_t pin, bool inputPullup) :
    _pin(pin),
    _inputPullup(inputPullup),
    _state(HIGH),
    _lastState(HIGH), 
    _pressedEvent(false),
    _releasedEvent(false),
    _lastDebounceTime(0), 
    _debounceDelay(50) 
{
    pinMode(_pin, _inputPullup ? INPUT_PULLUP : INPUT);
}

// Atualiza o estado do botão e detecta eventos de borda
void Button::update() {
    int reading = digitalRead(_pin);

    if (reading != _lastState) {
        _lastDebounceTime = millis(); // mudança detectada, zera o tempo
    }

    if ((millis() - _lastDebounceTime) > _debounceDelay) {
        if (reading != _state) {
            _state = reading;
            _pressedEvent = (_state == LOW);    // Botão pressionado
            _releasedEvent = (_state == HIGH);  // Botão liberado
        } 
    }
    _lastState = reading;
}

// Verifica se o botão foi pressionado
bool Button::wasPressed() {
    if(_pressedEvent){
        _pressedEvent = false; // Reseta o evento de pressionado após a leitura
        return true;
    }
    return false;
}

// Verifica se o botão está pressionado
bool Button::isPressed() {
    return _state == LOW;
}

// Verifica se o botão foi liberado
bool Button::wasReleased() {
    if(_releasedEvent){
        _releasedEvent = false; // Reseta o evento de liberado após a leitura
        return true;
    }
    return false;
}
