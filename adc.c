// -----------------------------------------------------------------------
//   Copyright (C) Rodrigo Almeida 2010
// -----------------------------------------------------------------------
//   Arquivo: adc.c
//            Biblioteca do conversor AD para o PIC18F4520
//   Autor:   Rodrigo Maximiano Antunes de Almeida
//            rodrigomax at unifei.edu.br
//   Licen?a: GNU GPL 2
// -----------------------------------------------------------------------
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; version 2 of the License.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
// -----------------------------------------------------------------------

#include "adc.h"
#include <pic18f4520.h>
#include "io.h"
#include "bits.h"

void adcInit(void) {
    //AN0-A0, AN1-A1 e AN2-A2 s?o anal?gicos e entradas
    pinMode(PIN_A0, INPUT);
    pinMode(PIN_A1, INPUT);
    //temperatura compartilhado com Display
    //removido da biblioteca
    // pinMode(PIN_A2, INPUT);

    bitSet(ADCON0, 0); //liga ADC
    //config an0-2 como anal?gico
    //ADCON1 = 0b00001100; //apenas AN0 ? analogico, a referencia ? baseada na fonte
    ADCON2 = 0b10101010; //FOSC /32, 12 TAD, Alinhamento ? direita e tempo de conv = 12 TAD

}

/*
int adcRead(unsigned int channel) {
    unsigned int ADvalor;
    ADCON0 &= 0b11000011; //zera os bits do canal
    if (channel < 3) {
        ADCON0 |= channel << 2;
    }
    
    ADCON0 |= 0b00000010; //inicia conversao

    while (bitTst(ADCON0, 1)); // espera terminar a convers?o;

    ADvalor = ADRESH; // le o resultado
    ADvalor <<= 8;
    ADvalor += ADRESL;
    return ADvalor;
}
 * */

// Fun??o personalizada
unsigned int adcRead(unsigned char chanell) {
    switch (chanell) {
        case 0:
            ADCON0 = 0x01;
            break;
        case 1:
            ADCON0 = 0x05;
            break;
        case 2:
            ADCON0 = 0x09;
            break;
    }

    ADCON0bits.GO = 1;
    while (ADCON0bits.GO == 1);

    return ((((unsigned int) ADRESH) << 2) | (ADRESL >> 6));
}