/*
 * File:   oscilador.c
 * Author: Luis Pablo Carranza
 *
 * Created on 24 de octubre de 2022, 11:37 PM
 */

#include "oscilador.h"

void setupINTOSC(uint8_t IRCF){
    if (IRCF == 7){
        OSCCONbits.IRCF = 0b111;    // Oscilador 8 MHz
    }
    if (IRCF == 6){
        OSCCONbits.IRCF = 0b110;    // Oscilador 4 MHz
    }
    if (IRCF == 5){
        OSCCONbits.IRCF = 0b101;    // Oscilador 2 MHz
    }
    if (IRCF == 4){
        OSCCONbits.IRCF = 0b100;    // Oscilador 1 MHz
    }
    if (IRCF == 3){
        OSCCONbits.IRCF = 0b011;    // Oscilador 500 KHz
    }
    if (IRCF == 2){
        OSCCONbits.IRCF = 0b010;    // Oscilador 250 KHz
    }
    if (IRCF == 1){
        OSCCONbits.IRCF = 0b001;    // Oscilador 125 KHz
    }
    if (IRCF == 0){
        OSCCONbits.IRCF = 0b000;    // Oscilador 31 KHz
    }
    
    OSCCONbits.SCS = 1;         // Oscilador interno
}
