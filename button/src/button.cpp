/* -----------------------------------------------------------------------
#   Copyright (C) 2025 Guilherme Fernandes
# -----------------------------------------------------------------------
#   Arquivo: button.cpp
#            Biblioteca para leitura de botões com debounce no Arduino
#   Autor:   Guilherme Fernandes de Oliveira
#            d2021005067 at unifei.edu.br
#   Licença: MIT License
# -----------------------------------------------------------------------
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# -----------------------------------------------------------------------
*/


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
