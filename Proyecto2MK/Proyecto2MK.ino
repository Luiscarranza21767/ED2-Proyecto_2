//***************************************************************************************************************************************
/* Universidad del Valle de Guatemala
 *  Proyecto de Electrónica Digital 2
 *  Video Juego 
 *  Mortal Kombat 
 *  Autores: Luis Pablo Carranza y Miguel Chacón
 *  Fecha de creación: 31/3/2023
 *  Última actualización: 27/04/2023
 *  
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

// Constante del color del fondo y de la posición vertical inicial de los jugadores
const int Backcolor = 0x28c3;
const int yp1init = 95;

//El SPI es el 0
//MOSI va a PA_5
//MISO va a PA_4
//SCK va a PA_2
//#define LCD_RST PD_0
//#define LCD_DC PD_1
//#define LCD_CS PA_3

// Variables para movimiento del jugador 1
int* P1_leftStatep;
int* P1_upStatep;
int* P1_rightStatep;
int* P1_hitStatep;
int* P1_lifep;

// Control de posición horizontal del jugador 1 y dirección (P1_lrp; 0 = derecha, 1 = izquierda)
int* xp1p;
boolean* P1_lrp;                   

// Variables para guardar un espacio para las tipo puntero
int P1_leftState = 1;
int P1_upState = 1;
int P1_rightState = 1;
int P1_hitState = 1;
int P1_life = 108;
int xp1 = 0;
boolean P1_lr = 0;

// Variables para el movimiento del jugador 2
int* P2_leftStatep;
int* P2_upStatep;
int* P2_rightStatep;
int* P2_hitStatep;
int* P2_lifep;

// Variables para control horizontal y de dirección del jugador 2 (P2_lrp; 0 = derecha, 1 = izquierda)
int* xp2p;
boolean* P2_lrp;                   // Variable para comparar dirección de p1 (1-left 0-right)

// Variables para guardar un espacio para las tipo puntero
int P2_leftState = 1;
int P2_upState = 1;
int P2_rightState = 1;
int P2_hitState = 1;
int P2_life = 108;
int xp2 = 280;
boolean P2_lr = 0;


// Variables para el menú y elección de los jugadores
int iniciomenu = 0;
int elegirmenu = 0;
int posP2 = 0;
int posP1 = 0;
int P1done = 0;
int P2done = 0;
// Variable para el ciclo del juego
int Game = 1;

// Variable para el control de las canciones del juego
int song = 0;

File myFile;

// 
int P1serial;
int P2serial;

int asciitohex(int val);
void mapeo_SD(char document[], int width, int height, int x0, int y0);
void checkbuttonP1(void);
void checkbuttonP2(void);
void Playermov(int* rightStatep, int* leftStatep, int* upStatep, int* hitStatep, boolean* lrp, uint8_t Walking[], uint8_t Jumping[], uint8_t Hitting[], uint8_t PbWalking[], int* xpp, int ypinit, int* xppb, int* pblifep, int Player);
void GameOver(uint8_t Win[], uint8_t Lose[], int* xpp, int* xppb, int ypinit, int player);
void VariablesReset(int* xp11, int* xp22, int* P1life, int* P2life, boolean* P2lr, boolean* P1lr);
extern uint8_t flecha[];
extern uint8_t Scorpion_Walking[];
extern uint8_t Scorpion_Jumping[];
extern uint8_t Scorpion_Hitting[];

extern uint8_t Subzero_Walking[];
extern uint8_t Subzero_Jumping[];
extern uint8_t Subzero_Hitting[];

extern uint8_t Subzero_Win[];
extern uint8_t Scorpion_Win[];
extern uint8_t Subzero_Lose[];
extern uint8_t Scorpion_Lose[];

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  
  P1_leftStatep = &P1_leftState;
  P1_upStatep = &P1_upState;
  P1_rightStatep = &P1_rightState;
  P1_hitStatep = &P1_hitState;
  P1_lifep = &P1_life;
  xp1p = &xp1;
  P1_lrp = &P1_lr;                   // Variable para comparar dirección de p1 (1-left 0-right)
  

  P2_leftStatep = &P2_leftState;
  P2_upStatep = &P2_upState;
  P2_rightStatep = &P2_rightState;
  P2_hitStatep = &P2_hitState;
  P2_lifep = &P2_life;
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

  // Envía la música de inicio
  song = 7;
  Serial2.write(song);
  Serial2.flush();
  
  mapeo_SD("InitMen.txt", 320, 210, 0, 0);
  mapeo_SD("2pgame.txt", 98, 8, 120, 210);
  
  delay(100);
  //LCD_Clear(0x00);
  LCD_Sprite(108, 208, 10, 10, flecha, 2, 0, 0, 1);
  
}
//***************************************************************************************************************************************
// Loop Infinito
//***************************************************************************************************************************************
void loop() {
  while (iniciomenu == 0){
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

  // Envía la música de pelea
  song = 8;
  Serial2.write(song);
  Serial2.flush();
  LCD_Clear(0x00);
  
  mapeo_SD("BackGam.txt", 320, 240, 0, 0);
  FillRect(10, 12, 110, 20, 0x6e6c);
  Rect(10, 12, 110, 20, 0x31a6);
  FillRect (200, 12, 110, 20, 0x6e6c);
  Rect(200, 12, 110, 20, 0x31a6);
  Game = 1;
  
  switch(posP1){
    case 1: 
      LCD_Print("SCORPION", 15, 16, 1, 0x0000, 0x6e6c);
      LCD_Print("SUB-ZERO", 205, 16, 1, 0x0000, 0x6e6c);
      LCD_Sprite(0, yp1init, 39, 77, Scorpion_Walking, 5, 0, 0, 0);
      LCD_Sprite(280, yp1init, 39, 77, Subzero_Walking, 5, 0, 1, 0);
    break;
    case 2:
      LCD_Print("SUB-ZERO", 15, 16, 1, 0x0000, 0x6e6c);
      LCD_Print("SCORPION", 205, 16, 1, 0x0000, 0x6e6c);
      LCD_Sprite(0, yp1init, 39, 77, Subzero_Walking, 5, 0, 0, 0);
      LCD_Sprite(280, yp1init, 39, 77, Scorpion_Walking, 5, 0, 1, 0);
    break;
    }

  
  while (Game == 1){
    int life2 = *P2_lifep;
    int life1 = *P1_lifep;
    
    switch(posP1){
      case 1:
        checkbuttonP1();  
        Playermov(&P1_rightState, &P1_leftState, &P1_upState, &P1_hitState, &P1_lr, Scorpion_Walking, 
        Scorpion_Jumping, Scorpion_Hitting, Subzero_Walking, &xp1, yp1init, &xp2, &P2_life, 1);
        checkbuttonP2();
        Playermov(&P2_rightState, &P2_leftState, &P2_upState, &P2_hitState, &P2_lr, Subzero_Walking, 
        Subzero_Jumping, Subzero_Hitting, Scorpion_Walking, &xp2, yp1init, &xp1, &P1_life, 2);
        if ((life2 <= 0) || (life1 <= 0)){
          if (life2 <= 0){
            GameOver(Scorpion_Win, Subzero_Lose, &xp1, &xp2, yp1init, 1);
            delay(500);
            mapeo_SD("Scwin.txt", 320, 240, 0, 0);
            LCD_Print("SCORPION GANA", 5, 200, 2, 0xffff, 0x0000);
            delay(3000);
          }
          else{
            GameOver(Subzero_Win, Scorpion_Lose, &xp2, &xp1, yp1init, 2);
            delay(500);
            mapeo_SD("Szwin.txt", 320, 240, 0, 0);
            LCD_Print("SUBZERO GANA", 5, 200, 2, 0xffff, 0x0000);
            delay(3000);
            }
          VariablesReset(&xp1, &xp2, &P1_life, &P2_life, &P2_lr, &P1_lr);
          }
        break;
      case 2:
        checkbuttonP1();  
        Playermov(&P1_rightState, &P1_leftState, &P1_upState, &P1_hitState, &P1_lr, Subzero_Walking, 
        Subzero_Jumping, Subzero_Hitting, Scorpion_Walking, &xp1, yp1init, &xp2, &P2_life, 1);
        checkbuttonP2();
        Playermov(&P2_rightState, &P2_leftState, &P2_upState, &P2_hitState, &P2_lr, Scorpion_Walking, 
        Scorpion_Jumping, Scorpion_Hitting, Subzero_Walking, &xp2, yp1init, &xp1, &P1_life, 2);
        if ((life2 <= 0) || (life1 <= 0)){
          if (life2 <= 0){
            GameOver(Subzero_Win, Scorpion_Lose, &xp1, &xp2, yp1init, 1);
            delay(500);
            mapeo_SD("Szwin.txt", 320, 240, 0, 0);
            LCD_Print("SUBZERO GANA", 5, 200, 2, 0xffff, 0x0000);
            delay(3000);
          }
          else{
            GameOver(Scorpion_Win, Subzero_Lose, &xp2, &xp1, yp1init, 2);
            delay(500);
            mapeo_SD("Scwin.txt", 320, 240, 0, 0);
            LCD_Print("SCORPION GANA", 5, 200, 2, 0xffff, 0x0000);
            delay(3000);
            }
          VariablesReset(&xp1, &xp2, &P1_life, &P2_life, &P2_lr, &P1_lr);
          }
        break;
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
    }
  else if (posP1 == 2){ //pos SUBZERO
        Rect(196, 106, 50, 50, 0x0000);
        Rect(86, 106, 50, 50, 0x0000);
        LCD_Bitmap(200, 110, 44, 44, scorpionpic);
        LCD_Bitmap(90, 110, 44, 44, subzeropic);
    }
}

void Playermov(int* rightStatep, int* leftStatep, int* upStatep, int* hitStatep, boolean* lrp, uint8_t Walking[], uint8_t Jumping[], uint8_t Hitting[], uint8_t PbWalking[], int* xpp, int ypinit, int* xppb, int* pblifep, int Player){

  int rightState = *rightStatep;
  int leftState = *leftStatep;
  int upState = *upStatep;
  int hitState = *hitStatep;
  boolean lr = *lrp;
  int xp = *xpp;
  int xpb = *xppb;
  int pblife = *pblifep;
  
    if(rightState == LOW){
      lr = 0;
      // Si superó el límite se queda en esa coordenada
      if((xp >= 280) || (xp == (xpb - 45))){
        if (xp >= 280){
          xp = 280;
        }
        // Animación del borde de la pantalla
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 3, 0, 0);
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 5, 0, 0);
        }
      else{
        if(xp < xpb){
          if(xp < (xpb-45)){
            xp += 5;
            }
          }
          else{
            xp += 5;
            }
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
      if((xp <= 0) || (xp == (xpb + 45))){
        if (xp  <= 0){
          xp = 0;
        }
        // Animación en el borde de la pantalla
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 3, 1, 0);
        delay(15);
        LCD_Sprite(xp, ypinit, 39, 77, Walking, 5, 5, 1, 0);
        }
      else{
        if(xp > xpb){
          if(xp >(xpb + 45)){
            xp -= 5;
            }
          }
        else{
          xp -= 5;
          }
        }
        
  
      delay(15);
      // Relleno para el rastro que deja el jugador
      for (int i = 39; i <= 53; i++){
      V_line(xp+i, ypinit - 1, 77, Backcolor);
      }
      // Animación del jugador 1
      int anim1 = (xp/17)%5;
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
      if(xp>=260){
        xp = 260;
        }
      if(!lr){
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 0, 0, 0);
        delay(25);
        if(xp < xpb){
          if(xp >= (xpb - 45)){
            if(xpb < 268){
              xpb += 12;
            }
            LCD_Sprite(xpb, ypinit, 39, 77, PbWalking, 5, 4, 1, 0);
            pblife -= (108/20);
            if(pblife > 0){
              switch(Player){
                case 1:
                  FillRect(200 + pblife, 14, 6, 17, 0xf800);
                  break;
                case 2:
                  FillRect(10 + pblife, 14, 6, 17, 0xf800);
                  break;
                }
              }
            }
          }
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 1, 0, 0);
        delay(25);
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 2, 0, 0);
        delay(25);
        for(int i = 1; i<=7; i++){
          V_line(xp-i, ypinit - 1, 77, Backcolor);
          }
        }        
      else{
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 0, 1, 0);
        delay(25);
        if(xp > xpb){
          if(xp <= (xpb + 45)){
            if (xpb > 8){
              xpb -= 8;
            }
            LCD_Sprite(xpb, ypinit, 39, 77, PbWalking, 5, 4, 0, 0);
            for(int i = 1; i<=14; i++){
              V_line(xp-i, ypinit - 1, 77, Backcolor);
              }
            pblife -= (108/20);
            if(pblife > 0){
              switch(Player){
                case 1:
                  FillRect(200 + pblife, 14, 6, 17, 0xf800);
                  break;
                case 2:
                  FillRect(10 + pblife, 14, 6, 17, 0xf800);
                  break;
                }
              }
            }
          }
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 1, 1, 0);
        delay(25);
        LCD_Sprite(xp, ypinit, 57, 77, Hitting, 3, 2, 1, 0);
        delay(25);
        for(int i = 1; i<=5; i++){
          V_line(xp-i, ypinit - 1, 77, Backcolor);
          }
        }
      }

    *lrp = lr;
    *xpp = xp;
    *xppb = xpb;
    *pblifep = pblife;
  }
  
void GameOver(uint8_t Win[], uint8_t Lose[], int* xpp, int* xppb, int ypinit, int player){
  
  int xp = *xpp;
  int xpb = *xppb;

  // Envía la música de final
  song = 9;
  Serial2.write(song);
  Serial2.flush();
  
  LCD_Bitmap(xp, yp1init, 39, 77, Win);
  if(player == 1){
    for(int i = 39; i<=46; i++){
      V_line(xp+i, ypinit - 1, 77, Backcolor);
      }
    }
  else{
    xpb -=16;
    for(int i = 39; i<=48; i++){
      V_line(xp+i, ypinit - 1, 77, Backcolor);
      }
    }
  LCD_Bitmap(xpb, yp1init + 44, 55, 33, Lose);
  FillRect(xpb, yp1init - 1, 55, 45, Backcolor); 
  
  }

  void VariablesReset(int* xp11, int* xp22, int* P1life, int* P2life, boolean* P2lr, boolean* P1lr){
    boolean P1lrx = *P1lr;
    boolean P2lrx = *P2lr;
    int xp1x = *xp11;
    int xp2x = *xp22;
    int p1lifex = *P1life;
    int p2lifex = *P2life;

    P1lrx = 0;
    P2lrx = 0;
    xp1x = 0;
    xp2x = 280;
    p1lifex = 108;
    p2lifex = 108;
    
    Game = 2;
    elegirmenu = 1;
    P1done = 0;
    P2done = 0;

    // Envía la música del menú
    song = 7;
    Serial2.write(song);
    Serial2.flush();
    
    *P1lr = P1lrx;
    *P2lr = P2lrx;
    *xp11 = xp1x;
    *xp22 = xp2x;
    *P1life = p1lifex;
    *P2life = p2lifex;
    }
