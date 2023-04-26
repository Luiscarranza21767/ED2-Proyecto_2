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
#include <SD.h>
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


#define SD_CS PB_6
const int Backcolor = 0x28c3;
const int yp1init = 95;

// El SPI es el 0
//MOSI va a PA_5
//MISO va a PA_4
//SCK va a PA_2
//#define LCD_RST PD_0
//#define LCD_DC PD_1
//#define LCD_CS PA_3

// Variables para jugador 1
int* P1_leftStatep;
int* P1_upStatep;
int* P1_rightStatep;
int* P1_hitStatep;

int* xp1p;
boolean* P1_lrp;                   // Variable para comparar dirección de p1 (1-left 0-right)

int P1_leftState = 1;
int P1_upState = 1;
int P1_rightState = 1;
int P1_hitState = 1;
int xp1 = 0;
boolean P1_lr = 0;

// Variables para jugador 2
int* P2_leftStatep;
int* P2_upStatep;
int* P2_rightStatep;
int* P2_hitStatep;

int* xp2p;
boolean* P2_lrp;                   // Variable para comparar dirección de p1 (1-left 0-right)

int P2_leftState = 1;
int P2_upState = 1;
int P2_rightState = 1;
int P2_hitState = 1;
int xp2 = 280;
boolean P2_lr = 0;


int iniciomenu = 0;
int elegirmenu = 0;
int posP2 = 0;
int posP1 = 0;
int P1done = 0;
int P2done = 0;
int song = 0;


File myFile;
int P1serial;
int P2serial;
int asciitohex(int val);
void mapeo_SD(char document[], int width, int height, int x0, int y0);
void checkbuttonP1(void);
void checkbuttonP2(void);
void Playermov(int* rightStatep, int* leftStatep, int* upStatep, int* hitStatep, boolean* lrp, uint8_t Walking[], uint8_t Jumping[], uint8_t Hitting[], int* xpp, int ypinit);

extern uint8_t flecha[];
extern uint8_t Scorpion_Walking[];
extern uint8_t Scorpion_Jumping[];
extern uint8_t Scorpion_Hitting[];

extern uint8_t Subzero_Walking[];
extern uint8_t Subzero_Jumping[];
extern uint8_t Subzero_Hitting[];

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  
  P1_leftStatep = &P1_leftState;
  P1_upStatep = &P1_upState;
  P1_rightStatep = &P1_rightState;
  xp1p = &xp1;
  P1_lrp = &P1_lr;                   // Variable para comparar dirección de p1 (1-left 0-right)

  P2_leftStatep = &P2_leftState;
  P2_upStatep = &P2_upState;
  P2_rightStatep = &P2_rightState;
  xp2p = &xp2;
  P2_lrp = &P2_lr;                   // Variable para comparar dirección de p2 (1-left 0-right)

  // Iniciar la comunicación serial
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial3.begin(9600);
  SPI.setModule(0);
  pinMode(SD_CS, OUTPUT);
  SD.begin(SD_CS);
  
  LCD_Init();
  LCD_Clear(0x00);
  delay(500);
  song = 1;
  Serial2.write(song);
  Serial2.flush();
  mapeo_SD("InitMen.txt", 320, 210, 0, 0);
  // Dibujar el fondo del juego (Ignorar el nombre xd)
//  
  mapeo_SD("2pgame.txt", 98, 8, 120, 210);
  delay(100);
  //LCD_Clear(0x00);
  
  
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  while (iniciomenu == 0){
      LCD_Sprite(108, 208, 10, 10, flecha, 2, 0, 0, 1);
      checkbuttonP1();
      if (P1_rightState == LOW || P1_leftState == LOW || P1_upState == LOW){
        iniciomenu = 1;
        elegirmenu = 1;
        delay(100);
      }
  }
  LCD_Clear(0x00);
  
  while(elegirmenu == 1){
    LCD_Print("Choose your fighter", 10, 50, 2, 0xffff, 0x0000);
    LCD_Print("Player 1", 20, 80, 2, 0xffff, 0x0000);
    LCD_Bitmap(90, 110, 44, 44, scorpionpic);
    LCD_Bitmap(200, 110, 44, 44, subzeropic);
    
    while (P1done == 0){
      P1selection();
    }
    
    while (P2done == 0){
      LCD_Print("   Your fighters   ", 10, 50, 2, 0xffff, 0x0000);
      LCD_Print("Player 2", 180, 80, 2, 0xffff, 0x0000);
      P2selection(); 
      P2done = 1; 
    }
    elegirmenu = 0;
    delay(1000);
  }
  song = 2;
  Serial2.write(song);
  Serial2.flush();
  LCD_Clear(0x00);
  mapeo_SD("BackGam.txt", 320, 240, 0, 0);
  while (1){
    switch(posP1){
      case 1:
        checkbuttonP1();  
        Playermov(&P1_rightState, &P1_leftState, &P1_upState, &P1_hitState, &P1_lr, Scorpion_Walking, Scorpion_Jumping, Scorpion_Hitting, &xp1, yp1init);
        checkbuttonP2();
        Playermov(&P2_rightState, &P2_leftState, &P2_upState, &P2_hitState, &P2_lr, Subzero_Walking, Subzero_Jumping, Subzero_Hitting, &xp2, yp1init);
        break;
      case 2:
        checkbuttonP1();  
        Playermov(&P1_rightState, &P1_leftState, &P1_upState, &P1_hitState, &P1_lr, Subzero_Walking, Subzero_Jumping, Subzero_Hitting, &xp1, yp1init);
        checkbuttonP2();
        Playermov(&P2_rightState, &P2_leftState, &P2_upState, &P2_hitState, &P2_lr, Scorpion_Walking, Scorpion_Jumping, Scorpion_Hitting, &xp2, yp1init);
    }
  }
}

// Función para convertir de ascii a hexadecimal
int asciitohex(int val){
  switch(val){
    case(48):
      return 0;
    case(49):
      return 1;
    case(50):
      return 2;
    case(51):
      return 3;
    case(52):
      return 4;
    case(53):
      return 5;
    case(54):
      return 6;
    case(55):
      return 7;
    case(56):
      return 8;
    case(57):
      return 9;
    case(97):
      return 0xa;
    case(98):
      return 0xb;
    case(99):
      return 0xc;
    case(100):
      return 0xd;
    case(101):
      return 0xe;
    case(102):
      return 0xf;
  
    }
  }

// Función para mapear imágenes de la SD
void mapeo_SD(char document[], int width, int height, int x0, int y0){
  //digitalWrite(SD_CS, LOW);
  myFile = SD.open(document);
  int yi = y0;
  int hex1 = 0;
  int val1 = 0;
  int val2 = 0;
  int mapear = 0;
  //int vertical = 0;
  width = width*2;
  unsigned char maps[width];
  if(myFile){
    while((myFile.available()) && (y0 < (height + yi))){
      mapear = 0;
      while(mapear<width){
        hex1 = myFile.read();
        if(hex1 == 120){
          val1 = myFile.read();
          val2 = myFile.read();
          val1 = asciitohex(val1);
          val2 = asciitohex(val2);
          maps[mapear] = val1*16 + val2;
          mapear++;
          }
        }
        LCD_Bitmap(x0, y0, (width/2), 1, maps);
        y0++;
      }
      myFile.close();
    }
  else{
    //Serial.println("No se pudo abrir la imagen");
    myFile.close();
    }
  }

void checkbuttonP1(void){
  while(Serial2.available()){
    P1serial = Serial2.read();
    switch(P1serial){
      case 1:
        P1_leftState = LOW;
        break;
      case 2:
        P1_upState = LOW;
        break;
      case 3:
        P1_rightState = LOW;
        break;
      case 4:
        P1_hitState = LOW;
        break;
        
      case 6:
        P1_leftState = HIGH;
        P1_rightState = HIGH;
        P1_upState = HIGH;
        P1_hitState = HIGH;
        break;
      }
    }
}

void checkbuttonP2(void){
  while(Serial3.available()){
    P2serial = Serial3.read();
    switch(P2serial){
      case 1:
        P2_leftState = LOW;
        break;
      case 2:
        P2_upState = LOW;
        break;
      case 3:
        P2_rightState = LOW;
        break;
      case 4:
        P2_hitState = LOW;
        break;
        
      case 6:
        P2_leftState = HIGH;
        P2_rightState = HIGH;
        P2_upState = HIGH;
        P2_hitState = HIGH;
        break;
      }
    }
}


void PICscorpion(){
  Rect(86, 106, 50, 50, 0x075b);
}

void PICsubzero(){
  Rect(196, 106, 50, 50, 0x075b);
}


void P1selection(){
  checkbuttonP1();
  if (P1_leftState == LOW){
    posP1--;
  }
  else if (P1_rightState == LOW){
    posP1++;
  }
  else if (P1_upState == LOW){
    P1done = 1;
  }

  if (posP1 == 1){ //pos SCORPION
      Rect(196, 106, 50, 50, 0x0000);
      PICscorpion();
    }
  else if (posP1 == 2){ //pos SUBZERO
    Rect(86, 106, 50, 50, 0x0000);
    PICsubzero();
    }
  else if (posP1 > 2){
    posP1 = 1;
  }
  else if (posP1 < 1){
    posP1 = 2;
  }
}

void P2selection(){
  
  if (posP1 == 1){ //pos SCORPION
      //Rect(196, 96, 50, 50, 0x0000);
    LCD_Bitmap(90, 110, 44, 44, scorpionpic);
    LCD_Bitmap(200, 110, 44, 44, subzeropic);
    //PICscorpion();
//    int eleccionP1 = 1;
//    int eleccionP2 = 2;
    }
  else if (posP1 == 2){ //pos SUBZERO
        Rect(196, 106, 50, 50, 0x0000);
        Rect(86, 106, 50, 50, 0x0000);
        LCD_Bitmap(200, 110, 44, 44, scorpionpic);
        LCD_Bitmap(90, 110, 44, 44, subzeropic);
//        int eleccionP1 = 2;
//        int eleccionP2 = 1;
    }
}

void Playermov(int* rightStatep, int* leftStatep, int* upStatep, int* hitStatep, boolean* lrp, uint8_t Walking[], uint8_t Jumping[], uint8_t Hitting[], int* xpp, int ypinit){

  int rightState = *rightStatep;
  int leftState = *leftStatep;
  int upState = *upStatep;
  int hitState = *hitStatep;
  boolean lr = *lrp;
  int xp = *xpp;
  
    if(rightState == LOW){
      lr = 0;
      // Si superó el límite se queda en esa coordenada
      if(xp >= 280){
        xp = 280;
        // Animación del borde de la pantalla
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 3, 0, 0);
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 5, 0, 0);
        }
      else{
        xp += 5;
        }
      delay(15);
      // Relleno de la parte que va dejando el jugador
      for(int i = 1; i<=5; i++){
        V_line(xp-i, ypinit - 1, 77, Backcolor);
      }
      
      // Animación del jugador
      int anim1 = (xp/17)%5;
      LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, anim1, 0, 0);
    }
  
    // Revisa si se presionó el control izquierdo del jugador 1
    if(leftState == LOW){
      lr = 1;          // Dirección izquierda
      // Revisa si llegó al borde de la pantalla
      if(xp <= 0){
        xp = 0;
        // Animación en el borde de la pantalla
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 3, 1, 0);
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 5, 1, 0);
        }
      else{
        xp -= 5;
        }
  
      delay(15);
      // Relleno para el rastro que deja el jugador
      for (int i = 39; i <= 53; i++){
      V_line(xp+i, ypinit - 1, 77, Backcolor);
      }
      // Animación del jugador 1
      int anim1 = (xp1/17)%5;
      LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, anim1, 1, 0);
      
      }
  
    // Revisa si presionó el botón de salto del jugador 1
    if(upState == LOW){
      int anim1y;
      // Revisa si el jugador se dirigía a la izquierda o derecha
      if (!lr){
        // Incrementa variable para la animación
        for (int yp = 1; yp <= 7; yp++){ 
          // Revisa si al mismo tiempo se presionó el botón del movimiento a la deracha
          if(rightState == LOW){
            // Si lo hizo entonces también aumenta la coordenada en xp1
            if(xp < 280){
            xp+=5;
            }
          }
          delay(25);
          // En las animaciones 3, 4 y 5 incrementa también la posición en y
          if((yp == 3) || (yp == 4) ||(yp == 5)){
            LCD_Sprite(xp, ypinit - yp*2, 40, 77, Jumping, 7, yp, 0, 0);
            }
          else{
            LCD_Sprite(xp, ypinit, 40, 77, Jumping, 7, yp, 0, 0);
            }
          // Dibuja la línea que borra el rastro del jugador
          for(int i = 1; i<=5; i++){
            V_line(xp-i, ypinit - 5, 82, Backcolor);
            }  
          }
        }
      else if(lr){
        for (int yp = 1; yp <= 7; yp++){ 
          // Revisa si se presionaba el botón del movimiento a la izquierda
          if(leftState == LOW){
            if(xp > 0){
            // Si lo hacía disminuye la posición en x
            xp-=5;
            }
          }
          delay(25);
          // Para animaciones 3, 4 y 5 se sube la posición en y
          if((yp == 3) || (yp == 4) ||(yp == 5)){
            LCD_Sprite(xp, ypinit - yp*2, 40, 77, Jumping, 7, yp, 1, 0);
            }
          else{
            LCD_Sprite(xp, ypinit, 40, 77, Jumping, 7, yp, 1, 0);
            }
          // Dibuja la línea que borra el rastro del jugador
          for (int i = 39; i <= 43; i++){
            V_line(xp+i, ypinit - 5, 82, Backcolor);
            }
        }
      }
    }
    if(hitState == LOW){
      if(!lr){
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 0, 0, 0);
        delay(25);
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 1, 0, 0);
        delay(25);
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 2, 0, 0);
        delay(25);
        for(int i = 1; i<=7; i++){
          V_line(xp-i, ypinit - 1, 77, Backcolor);
          }
        for(int i = 1; i<=20; i++){
          V_line(xp+35, ypinit - 1, 77, Backcolor);
          }
        }
        
      else{
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 0, 1, 0);
        delay(25);
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 1, 1, 0);
        delay(25);
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 2, 1, 0);
        delay(25);
        for(int i = 1; i<=10; i++){
          V_line(xp-i, ypinit - 1, 77, Backcolor);
          }
        for(int i = 1; i<=7; i++){
          V_line(xp+55, ypinit - 1, 77, Backcolor);
          }
        }
      
        
      }
    *lrp = lr;
    *xpp = xp;
  }
