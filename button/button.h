/* -----------------------------------------------------------------------
#   Copyright (C) 2025 Guilherme Fernandes
# -----------------------------------------------------------------------
#   Arquivo: button.h
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