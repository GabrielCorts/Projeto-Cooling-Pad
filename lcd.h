// -----------------------------------------------------------------------
//   Copyright (C) Rodrigo Almeida 2010
// -----------------------------------------------------------------------
//   Arquivo: lcd.h
//            Header da biblioteca de manipulação do LCD
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

#ifndef LCD_H
	#define LCD_H

    #define CLR 0X01
    #define ON 0X0F
    #define OFF 0x0C

	void lcdCommand(unsigned char cmd);
	void lcdData(unsigned char valor);
    void lcdInit(void);
    void lcdPosition(unsigned char linha, unsigned char coluna);
    void lcdString(char string[]);
    void lcdLongString(char string[], unsigned char linha);
        
#endif