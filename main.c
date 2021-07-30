/*
 * File:   main.c
 * Author: Gabriel Cortes Cassiano Pereira
 *
 * Created on 21 de Julho de 2021, 14:17
 */


#include <xc.h>
#include "lcd.h"
#include "keypad.h"
#include "bits.h"
#include "itoa.h"
#include "adc.h"
#include "pwm.h"
#include "ssd.h"

unsigned char mode;

void changeMode(unsigned char i);
void updateLCDMode(void);

void main(void) {
    // Variáveis
    unsigned int tecla = 16, tmpi, tmpiAux;
    unsigned short int i;
    float calc;
    unsigned char str[6], dutyCycle, setPoint, cont;

    //Inicialização---------------------------------------------------------------------------
    lcdInit();
    kpInit();
    ssdInit();
    ADCON1 = 0x00; //Configura todos AN2 a AN0 como entrada analógica (só precisa do AN2)
    TRISCbits.TRISC5 = 0; // Configura pino do heater como saída
    TRISCbits.TRISC2 = 0; // Configura pino do cooler como saída

    lcdPosition(0, 0);
    lcdString("Selecione um\0");
    lcdPosition(1, 0);
    lcdString("  modo     5-OK\0");

    while (1) {
        kpDebounce();
        if (bitTst(kpRead(), 6)) {
            tecla = kpRead();
            break;
        }
    }

    mode = 0;
    lcdCommand(CLR);
    lcdPosition(0, 0);
    lcdString("4 <- Manual -> 6\0");
    lcdPosition(1, 0);
    lcdString(" 5-OK\0");

    //Loop principal------------------------------------------------------------------------------
    for (;;) {
        kpDebounce();

        if (kpRead() != tecla) {
            tecla = kpRead();
            if (bitTst(tecla, 2)) {
                changeMode(0);
                updateLCDMode();
            }
            if (bitTst(tecla, 10)) {
                changeMode(1);
                updateLCDMode();
            }
            if (bitTst(tecla, 6)) {
                switch (mode) {
                    case 0: //Modo controle manual---------------------------------------------
                        //--------------------------------------------------------------------------------------------------------
                        ADCON0bits.ADON = 1; // Habilita conversor A/D
                        PORTCbits.RC2 = 1; //Habilita o cooler
                        pwmInit(); // Habilita PWM
                        pwmSet(0); // Mantém cooler desligado
                        lcdCommand(CLR);
                        lcdPosition(1, 0);
                        lcdString(" 7-Voltar  ?-9\0");

                        while (1) {
                            kpDebounce();
                            ssdUpdate();
                            //Lê o valor analógico do potenciômetro---------------------
                            tmpi = adcRead(0); //MAX = 1023, MIN = 0
                            //-------------------------------------------------------

                            if (tmpi != tmpiAux) {
                                ssdDigit(16, 1);
                                if (tmpi > 999) {
                                    ssdDigit(9, 2);
                                    ssdDigit(9, 3);
                                } else {
                                    ssdDigit(tmpi / 100, 2);
                                    ssdDigit((tmpi / 10) % 10, 3);
                                }

                                pwmSet(tmpi / 10);
                                tmpiAux = tmpi;
                            }

                            //Lê o valor da temperatura do LM35---------------------
                            tmpi = adcRead(2) * 10 / 2;
                            itoa(tmpi, str);
                            //-------------------------------------------------------

                            if (kpRead() != tecla) {
                                tecla = kpRead();

                                if (bitTst(tecla, 1)) {
                                    pwmEnd(); // Desabilita PWM
                                    PORTCbits.RC2 = 0; // Desliga o cooler
                                    PORTCbits.RC5 = 0; // Desliga o heater
                                    ssdEnd(); //Desliga displays
                                    ADCON0 = 0x00; // Desabilita AD e desliga todos canais

                                    break;
                                }

                                if (bitTst(tecla, 5)) {
                                    bitFlp(PORTC, 5); //Liga/desliga o heater
                                }

                                if (bitTst(tecla, 9)) {
                                    ssdEnd(); //Desliga displays
                                    lcdCommand(CLR);
                                    lcdLongString("Utilize o potenciometro P1\0", 0x80);
                                    lcdLongString("para regular a velocidade\0", 0xC0);
                                    lcdCommand(CLR);
                                    lcdPosition(1, 0);
                                    lcdString(" 7-Voltar  ?-9\0");
                                }
                            }

                            lcdPosition(0, 0);
                            lcdString(" Temp.  \0");
                            lcdData(str[2]);
                            lcdData(str[3]);
                            lcdData(',');
                            lcdData(str[4]);
                            lcdData('o');
                            lcdData('C');
                        }
                        break;
                    case 1: //Modo resfriamento automático---------------------------------------------
                        //--------------------------------------------------------------------------------------------------------
                        lcdPosition(0, 0);
                        lcdString("Pressione 4/6 p/\0");
                        lcdPosition(1, 0);
                        lcdString("definir a tmp.\0");

                        ADCON0bits.ADON = 1; // Habilita conversor A/D
                        PORTCbits.RC2 = 1; //Habilita o cooler
                        pwmInit(); // Habilita PWM
                        pwmSet(0); // Mantém cooler desligado
                        lcdCommand(CLR);
                        lcdPosition(1, 0);
                        lcdString(" 7-Voltar  ?-9\0");

                        while (1) {
                            kpDebounce();
                            ssdUpdate();
                            //Lê o valor analógico do potenciômetro---------------------
                            tmpi = adcRead(0); //MAX = 1023, MIN = 0
                            setPoint = (tmpi / 25) + 40;
                            if (setPoint > 80)
                                setPoint = 80;
                            ssdDigit(setPoint / 10, 1);
                            ssdDigit(setPoint % 10, 2);
                            ssdDigit(17, 3);
                            //-------------------------------------------------------

                            //Lê o valor da temperatura do LM35---------------------
                            tmpi = adcRead(2) * 10 / 2;
                            itoa(tmpi, str);
                            tmpi = tmpi / 10; //Converte pra grau celsius perdendo a virgula
                            //-------------------------------------------------------

                            if (tmpi >= 30) {
                                //Cálculo para acelerar o cooler gradualmente
                                //Quanto maior o SP menos velocidade ele precisa pra estabilizar
                                //REVISAR OS CÁLCULOS (talvez usar switch case pra configurar um por um)
                                //calc = ((tmpi - 30 - ((setPoint - 5) / 5) * ((setPoint - 30) / 20.0)) / ((setPoint - 30) / 20.0)) * 5;
                                calc = (160 - 2*setPoint) * ((float) tmpi/ (float) setPoint);
                                if (dutyCycle != calc) {
                                    dutyCycle = (char) calc;
                                    pwmSet(dutyCycle); //Ajusta ciclo de trabalho do PWM
                                }
                            }

                            if (kpRead() != tecla) {
                                tecla = kpRead();

                                if (bitTst(tecla, 1)) {
                                    pwmEnd(); // Desabilita PWM
                                    PORTCbits.RC2 = 0; // Desliga o cooler
                                    PORTCbits.RC5 = 0; // Desliga o heater
                                    ssdEnd(); //Desliga displays
                                    ADCON0 = 0x00; // Desabilita AD e desliga todos canais

                                    break;
                                }

                                if (bitTst(tecla, 5)) {
                                    bitFlp(PORTC, 5); //Liga/desliga o heater
                                }

                                if (bitTst(tecla, 9)) {
                                    ssdEnd(); //Desliga displays
                                    lcdCommand(CLR);
                                    lcdLongString("Utilize o potenciometro P1 para\0", 0x80);
                                    lcdLongString("definir o set point de temperatura\0", 0xC0);
                                    lcdCommand(CLR);
                                    lcdPosition(1, 0);
                                    lcdString(" 7-Voltar  ?-9\0");
                                }
                            }

                            lcdPosition(0, 0);
                            lcdString(" Temp.  \0");
                            lcdData(str[2]);
                            lcdData(str[3]);
                            lcdData(',');
                            lcdData(str[4]);
                            lcdData('o');
                            lcdData('C');
                        }
                        break;
                    case 2: //Modo cooler desligado---------------------------------------------------
                        //--------------------------------------------------------------------------------------------------------
                        ADCON0bits.ADON = 1; // Habilita conversor A/D
                        ADCON0bits.CHS1 = 1; //Seleciona AN2
                        TRISD = 0X00; // Configura tris D como saída para ligar LEDs
                        PORTD = 0x00;
                        lcdCommand(CLR);
                        lcdPosition(1, 0);
                        lcdString(" 7-Voltar  \0");

                        while (1) {
                            cont++;
                            kpDebounce();
                            if (cont > 20) {
                                if (bitTst(PORTD, 0))
                                    PORTD = 0x80;
                                else
                                    PORTD >>= 1;
                                PORTB = PORTD;
                                cont = 0;
                            }
                            //Lê o valor da temperatura do LM35---------------------
                            tmpi = adcRead(2) * 10 / 2;
                            itoa(tmpi, str);
                            //-------------------------------------------------------

                            if (kpRead() != tecla) {
                                tecla = kpRead();

                                if (bitTst(tecla, 1)) {
                                    PORTCbits.RC5 = 0; // Desliga o heater
                                    ADCON0 = 0x00; // Desabilita AD e desliga todos canais
                                    break;
                                }

                                if (bitTst(tecla, 5)) {
                                    bitFlp(PORTC, 5); //Liga/desliga o heater
                                    if (PORTCbits.RC5)
                                        PORTD = 0x80;
                                    if (!PORTCbits.RC5)
                                        PORTD = 0X00;
                                    PORTB = PORTD;
                                }
                            }

                            lcdPosition(0, 0);
                            lcdString(" Temp.  \0");
                            lcdData(str[2]);
                            lcdData(str[3]);
                            lcdData(',');
                            lcdData(str[4]);
                            lcdData('o');
                            lcdData('C');
                        }
                        break;
                }

                updateLCDMode();
            }
        }


    };
    return;
}

void changeMode(unsigned char i) {
    if (!i) {
        if (!mode)
            mode = 2;
        else
            mode--;
    }

    if (i) {
        if (mode == 2)
            mode = 0;
        else
            mode++;
    }

    return;
}

void updateLCDMode(void) {
    switch (mode) {
        case 0:
            lcdCommand(CLR);
            lcdPosition(0, 0);
            lcdString("4 <- Manual -> 6\0");
            lcdPosition(1, 0);
            lcdString(" 5-OK\0");
            break;
        case 1:
            lcdCommand(CLR);
            lcdPosition(0, 0);
            lcdString("4 <-  Auto. -> 6\0");
            lcdPosition(1, 0);
            lcdString(" 5-OK\0");
            break;
        case 2:
            lcdCommand(CLR);
            lcdPosition(0, 0);
            lcdString("4 <- Deslig -> 6\0");
            lcdPosition(1, 0);
            lcdString(" 5-OK\0");
            break;
    }

    return;
}