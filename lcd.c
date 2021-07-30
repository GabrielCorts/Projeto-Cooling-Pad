// -----------------------------------------------------------------------
//   Copyright (C) Rodrigo Almeida 2010
// -----------------------------------------------------------------------
//   Arquivo: lcd.c
//            Biblioteca de manipulação do LCD
//   Autor:   Rodrigo Maximiano Antunes de Almeida
//            rodrigomax at unifei.edu.br
//   Licença: GNU GPL 2
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


#include "lcd.h"
#include <pic18f4520.h>
#include "bits.h"
#include "io.h"


#define EN PIN_E1
#define RS PIN_E2
#define L0 0X80
#define L1 0XC0

void Delay40us(void) {
    unsigned char i;
    for (i = 0; i < 25; i++); //valor aproximado
}

void Delay2ms(void) {
    unsigned char i;
    for (i = 0; i < 50; i++) {
        Delay40us();
    }
}

void delay(void) {
    int i, j;
    for (i = 0; i < 15; i++)
        for (j = 0; j < 1000; j++);
    return;
}

void lcdCommand(unsigned char cmd) {
    unsigned char old_D;
    old_D = PORTD;

    //garantir compatibilidade
    TRISD = 0x00;

    digitalWrite(RS, LOW); //comando
    PORTD = cmd;

    digitalWrite(EN, HIGH); //Pulso no Enable
    digitalWrite(EN, LOW);


    PORTD = old_D;

    if ((cmd == 0x02) || (cmd == 0x01)) {
        Delay2ms();
    } else {
        Delay40us();
    }


}

void lcdData(unsigned char valor) {
    //garantir compatibilidade
    unsigned char old_D;
    old_D = PORTD;

    TRISD = 0x00;
    digitalWrite(RS, HIGH); //comando

    PORTD = valor;

    digitalWrite(EN, HIGH); //Pulso no Enable
    digitalWrite(EN, LOW);

    PORTD = old_D;

    Delay40us();

}

void lcdInit(void) {
    // configurações de direção dos terminais
    pinMode(RS, OUTPUT);
    pinMode(EN, OUTPUT);
    TRISD = 0x00; //dados

    // garante inicialização do LCD (+-10ms)
    Delay2ms();
    Delay2ms();
    Delay2ms();
    Delay2ms();
    Delay2ms();
    //precisa enviar 4x pra garantir 8 bits
    lcdCommand(0x38); //8bits, 2 linhas, 5x8
    Delay2ms();
    Delay2ms();
    lcdCommand(0x38); //8bits, 2 linhas, 5x8
    Delay2ms();
    lcdCommand(0x38); //8bits, 2 linhas, 5x8

    lcdCommand(0x38); //8bits, 2 linhas, 5x8
    lcdCommand(0x06); //modo incremental

    //habilitar o cursor, trocar 0x0C por 0x0F;
    lcdCommand(0x0C); //display e cursor on, com blink
    lcdCommand(0x01); //limpar display
}

// Funções extras

void lcdPosition(unsigned char linha, unsigned char coluna) {

    unsigned char comando;

    switch (linha) {
        case 0:
            comando = L0 + coluna;
            break;
        case 1:
            comando = L1 + coluna;
            break;
        default:
            comando = L0;
    }

    lcdCommand(comando);
}

void lcdString(char string[]) {
    unsigned char end = 0, cont = 0;
    while (!end) {
        lcdData(string[cont]);
        cont++;
        if (string[cont] == '\0')
            end = 1;
    }
}

void lcdLongString(char string[], unsigned char linha) {
    unsigned char cont = 15, i;
    while (cont > 0) {
        lcdCommand(linha + cont);
        for (i = 0; i < 16 - cont; i++) {
            lcdData(string[i]);
        }
        cont--;
        delay();
    }

    cont = 0;

    while (string[cont] != '\0') {
        lcdCommand(linha);
        i = cont;
        while (string[i] != '\0') {
            lcdData(string[i]);
            i++;
        }
        if (string[i] == '\0')
            lcdData(' ');
        cont++;
        delay();
    }
    
    lcdCommand(linha);
    lcdData(' ');
}