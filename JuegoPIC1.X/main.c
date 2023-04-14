//CODIGO PIC SLAVE 1 VIDEOJUEGO


//*****************************************************************************
// Palabra de configuración
//*****************************************************************************
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (RCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, RC on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

//*****************************************************************************
// Definición e importación de librerías
//*****************************************************************************
#include <xc.h>
#include "oscilador.h"
#include <stdlib.h>
#include <string.h>

//*****************************************************************************
// Definición de variables
//*****************************************************************************
#define _XTAL_FREQ 500000 //kHZ

uint8_t accion;
//*****************************************************************************
// Definición de funciones para que se puedan colocar después del main de lo 
// contrario hay que colocarlos todas las funciones antes del main
//*****************************************************************************
void setup(void);
void initUART(void);    //se inicia la comunicación serial

//*****************************************************************************
// Código de Interrupción 
//*****************************************************************************
void __interrupt() isr(void){
   
    //VALORES A ENVIAR POR CADA BOTON
   	if (RBIF == 1){
        if (PORTBbits.RB0 == 0){
            __delay_ms(10);

            //ACCION DEL PUERTO
            accion = 1;  //ACCION: 
            INTCONbits.RBIF = 0;
            TXREG = accion;

        }
        else if (PORTBbits.RB1 == 0){
            __delay_ms(10);

            //ACCION DEL PUERTO
            accion = 2; //ACCION: 
            INTCONbits.RBIF = 0;
            TXREG = accion;

        }

        else if (PORTBbits.RB2 == 0){
            __delay_ms(10);

            //ACCION DEL PUERTO
            accion = 3; //ACCION: 
            INTCONbits.RBIF = 0;
            TXREG = accion;

        }

        else if (PORTBbits.RB3 == 0){
            __delay_ms(10);

            //ACCION DEL PUERTO
            accion = 4; //ACCION: 
            INTCONbits.RBIF = 0;
            TXREG = accion;

        }
        else{
            accion = 6;
            TXREG = accion;
        }
    
    }
}
//*****************************************************************************
// Main
//*****************************************************************************
void main(void) {
    setupINTOSC(3);
    setup();
    initUART();
    while(1){
     ;   
    }
}

//******************************************************************************
// Función para configurar GPIOs
//******************************************************************************
void setup(void){
    //PUERTOS
    ANSEL = 0;
    ANSELH = 0;
    TRISB = 0b00001111;
    PORTB = 0;
    //Interrupciones
    INTCONbits.RBIE = 1; 
    INTCONbits.RBIF = 0;
    INTCONbits.GIE = 1; //interrupciones globales
    //Pullups de puertos
    WPUBbits.WPUB0 = 1; //inputs RB0
    IOCBbits.IOCB0 = 1; //inputs RB0
    WPUBbits.WPUB1 = 1; //inputs RB1
    IOCBbits.IOCB1 = 1; //inputs RB1
    WPUBbits.WPUB2 = 1; //inputs RB2
    IOCBbits.IOCB2 = 1; //inputs RB2  
    WPUBbits.WPUB3 = 1; //inputs RB3
    IOCBbits.IOCB3 = 1; //inputs RB3
    //PULLUPS INTERNOS
    OPTION_REGbits.nRBPU = 0; //no RBPU, habilitan los pullups internos
    //I2C
    //I2C_Master_Init(100000); //Se inicializa la comunicación I2C
    
}

//******************************************************************************
// Función para configurar UART
//******************************************************************************
void initUART(void){
    // Paso 1: configurar velocidad baud rate
    SPBRG = 12;
    // Paso 2:
    TXSTAbits.SYNC = 0;         // Trabajaremos de forma asincrona
    RCSTAbits.SPEN = 1;         // habilitamos módulo UART
    // Paso 3:
    TXSTAbits.BRGH = 1;         //baud rate high
    BAUDCTLbits.BRG16 = 1;      //16 bit baud rate generator
    // Paso 4:
    TXSTAbits.TXEN = 1;         // Habilitamos la transmision
    PIR1bits.TXIF = 0;
    RCSTAbits.CREN = 1;         // Habilitamos la recepcion   
}