//***************************************************************************************************************************************
/* Librería para el uso de la pantalla ILI9341 en modo SPI
 * Basado en el código de martinayotte - https://www.stm32duino.com/viewtopic.php?t=637
 * Adaptación, migración y creación de nuevas funciones: Pablo Mazariegos y José Morales
 * Con ayuda de: José Guerra
 * Adaptación del ejemplo a librería: Luis Pablo Carranza
 * IE3027: Electrónica Digital 2 - 2023
 * Laboratorio 7 Luis Pablo Carranza
 */
//***************************************************************************************************************************************
#include <stdint.h>
#include <ili9341_SPI.h>
#include <SPI.h>
#include "bitmaps.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include <avr/pgmspace.h>

#define P1_left PC_7
#define P1_up PC_6
#define P1_down PC_5
#define P1_right PC_4

const int Backcolor = 0x28c3;
// El SPI es el 0
//MOSI va a PA_5
//MISO va a PA_4
//SCK va a PA_2

int P1_leftState = 1;
int P1_upState = 1;
int P1_downState = 1;
int P1_rightState = 1;

int xp1 = 0;
int P1_comp_left = 0;                   // Variable para comparar posición del jugador 1
int P1_comp_right = 39;                 // Variable para comparar posición del jugador 1
//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  
  pinMode(P1_left, INPUT_PULLUP);
  pinMode(P1_up, INPUT_PULLUP);
  pinMode(P1_down, INPUT_PULLUP);
  pinMode(P1_right, INPUT_PULLUP);
  
  Serial.begin(9600);
  SPI.setModule(0);
  LCD_Init();
  LCD_Clear(0x00);
  delay(500);
  extern uint8_t Background_underfloor[];
  extern uint8_t Background_roof[];
  extern uint8_t Background_floor[];
  extern uint8_t Background_riels[];
  
  // Dibujo del fondo
  FillRect(0, 65, 320, 114, Backcolor);
  LCD_Bitmap(0, 205, 320, 36, Background_riels);
  
  LCD_Bitmap(0, 0, 143, 65, Background_roof);
  LCD_Bitmap(143, 0, 143, 65, Background_roof);
  for(int i = 0; i<= 55; i++){
    LCD_Sprite(285 + i, 0, 1, 65, Background_roof, 143, i+1, 0, 0);
    }

  LCD_Bitmap(0, 179, 143, 9, Background_floor); 
  LCD_Bitmap(143, 179, 143, 9, Background_floor);
  for(int i = 0; i<=55; i++){
    LCD_Sprite(285 + i, 179, 1, 9, Background_floor, 143, i+1, 0, 0);
    }
  
  for(int i = 0; i <= 331; i++){
    LCD_Bitmap(i, 188, 17, 17, Background_underfloor);
    i+=16;
    } 
  
  
  
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  
  P1_leftState = digitalRead(P1_left);
  P1_rightState = digitalRead(P1_right);
  P1_upState = digitalRead(P1_up);
  extern uint8_t Scorpion_Walking[];
  extern uint8_t Scorpion_Jumping[];
  if(P1_rightState == LOW){
    // Si superó el límite redibuja la última parte
    if(xp1 >= 280){
//      FillRect(276, 102, 45, 77, Backcolor);
      xp1 = 280;
      delay(15);
      LCD_Sprite(xp1, 102, 39, 77, Scorpion_Walking, 5, 3, 0, 0);
      delay(15);
      LCD_Sprite(xp1, 102, 39, 77, Scorpion_Walking, 5, 5, 0, 0);
      }
    else{
      xp1 += 5;
      }
    delay(15);
    // Relleno de la parte que va dejando el jugador
    for(int i = 1; i<=5; i++){
      V_line(xp1-i, 101, 77, Backcolor);
    }
    // Animación del jugador
    int anim1 = (xp1/15)%2;
    LCD_Sprite(xp1, 102, 39, 77, Scorpion_Walking, 5, anim1, 0, 0);
    
    
  }
  
  if(P1_leftState == LOW){
    if(xp1 <= 0){
      xp1 = 0;
      delay(15);
      LCD_Sprite(xp1, 102, 39, 77, Scorpion_Walking, 5, 3, 1, 0);
      delay(15);
      LCD_Sprite(xp1, 102, 39, 77, Scorpion_Walking, 5, 5, 1, 0);
//      FillRect(0, 102, 45, 77, Backcolor);
      }
    else{
      xp1 -= 5;
      }

    delay(15);
    for (int i = 39; i <= 43; i++){
    V_line(xp1+i, 101, 77, Backcolor);
    }
    int anim1 = (xp1/15)%2;
    LCD_Sprite(xp1, 102, 39, 77, Scorpion_Walking, 5, anim1, 1, 0);
    
    }
  if(P1_upState == LOW){
    int anim1y;
    for (int yp1 = 0; yp1 <= 4; yp1++){ 
      anim1y = (yp1/15)%2;
      LCD_Sprite(xp1, 102 - yp1, 40, 77, Scorpion_Jumping, 7, anim1y, 2, 0);
      }
    }
}
