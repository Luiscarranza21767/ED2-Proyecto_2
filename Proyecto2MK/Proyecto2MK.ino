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

#define P1_left PC_7
#define P1_up PC_6
#define P1_down PC_5
#define P1_right PC_4

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

boolean P1_leftState = 1;
boolean P1_upState = 1;
boolean P1_downState = 1;
boolean P1_rightState = 1;

int xp1 = 0;
boolean P1_lr = 0;                   // Variable para comparar dirección de p1 (1-left 0-right)
int iniciomenu = 0;
int elegirmenu = 0;
int posP2 = 0;
int posP1 = 0;
int P1done = 0;
int P2done = 0;


File myFile;
int P1serial;
int asciitohex(int val);
void mapeo_SD(char document[], int width, int height, int x0, int y0);
void checkbuttonP1(void);
void saveImage(char doc[]);

extern uint8_t flecha[];

//***************************************************************************************************************************************
// Inicialización
//***************************************************************************************************************************************
void setup() {
  SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
  
  pinMode(P1_left, INPUT_PULLUP);
  pinMode(P1_up, INPUT_PULLUP);
  pinMode(P1_down, INPUT_PULLUP);
  pinMode(P1_right, INPUT_PULLUP);

  // Iniciar la comunicación serial
  Serial.begin(9600);
  SPI.setModule(0);
  Serial.print("Inicializando la tarjeta SD");
  pinMode(SD_CS, OUTPUT);
  if(!SD.begin(SD_CS)){
    Serial.println("Inicialización fallida");
    return;
    }
    Serial.println("Inicialización completa");
  Serial2.begin(9600);
  // Iniciar la comunicación con la pantalla
  LCD_Init();
  LCD_Clear(0x00);
  delay(500);

//  saveImage("InitMen.txt");
  
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
      //Stleft = digitalRead(SWLEFT);
      if (P1_rightState == LOW || P1_leftState == LOW || P1_upState == LOW){
        iniciomenu = 1;
        elegirmenu = 1;
        delay(100);
      }
  }
  LCD_Clear(0x00);
  
  while(elegirmenu == 1){
    LCD_Print("Choose your fighter", 10, 50, 2, 0xffff, 0x0000);
    LCD_Print("Player 1", 30, 70, 2, 0xffff, 0x0000);
    LCD_Bitmap(90, 100, 44, 44, scorpionpic);
    LCD_Bitmap(200, 100, 44, 44, subzeropic);
    
    while (P1done == 0){
      verifposP1();
      eleccionP1();
    }
    
    while (P2done == 0){
      LCD_Print("   Your fighters   ", 10, 50, 2, 0xffff, 0x0000);
      LCD_Print("Player 2", 160, 70, 2, 0xffff, 0x0000);
      verifposP2(); 
      P2done = 1; 
    }
    elegirmenu = 0;
    delay(1000);
  }
  LCD_Clear(0x00);
  mapeo_SD("BackGam.txt", 320, 240, 0, 0);
  while (1){
    checkbuttonP1();  
    
    extern uint8_t Scorpion_Walking[];
    extern uint8_t Scorpion_Jumping[];
  
    // Revisa si se presionó el control derecho del jugador 1
    if(P1_rightState == LOW){
      P1_lr = 0;
      // Si superó el límite se queda en esa coordenada
      if(xp1 >= 280){
        xp1 = 280;
        // Animación del borde de la pantalla
        delay(15);
        LCD_Sprite(xp1, yp1init, 39, 77, Scorpion_Walking, 5, 3, 0, 0);
        delay(15);
        LCD_Sprite(xp1, yp1init, 39, 77, Scorpion_Walking, 5, 5, 0, 0);
        }
      else{
        xp1 += 5;
        }
      delay(15);
      // Relleno de la parte que va dejando el jugador
      for(int i = 1; i<=5; i++){
        V_line(xp1-i, yp1init - 1, 77, Backcolor);
      }
      // Animación del jugador
      int anim1 = (xp1/17)%5;
      LCD_Sprite(xp1, yp1init, 39, 77, Scorpion_Walking, 5, anim1, 0, 0);
    }
  
    // Revisa si se presionó el control izquierdo del jugador 1
    if(P1_leftState == LOW){
      P1_lr = HIGH;          // Dirección izquierda
      // Revisa si llegó al borde de la pantalla
      if(xp1 <= 0){
        xp1 = 0;
        // Animación en el borde de la pantalla
        delay(15);
        LCD_Sprite(xp1, yp1init, 39, 77, Scorpion_Walking, 5, 3, 1, 0);
        delay(15);
        LCD_Sprite(xp1, yp1init, 39, 77, Scorpion_Walking, 5, 5, 1, 0);
        }
      else{
        xp1 -= 5;
        }
  
      delay(15);
      // Relleno para el rastro que deja el jugador
      for (int i = 39; i <= 43; i++){
      V_line(xp1+i, yp1init - 1, 77, Backcolor);
      }
      // Animación del jugador 1
      int anim1 = (xp1/17)%5;
      LCD_Sprite(xp1, yp1init, 39, 77, Scorpion_Walking, 5, anim1, 1, 0);
      
      }
  
    checkbuttonP1();   
    // Revisa si presionó el botón de salto del jugador 1
    if(P1_upState == LOW){
      int anim1y;
      // Revisa si el jugador se dirigía a la izquierda o derecha
      if (!P1_lr){
        // Incrementa variable para la animación
        for (int yp1 = 1; yp1 <= 7; yp1++){ 
          // Revisa si al mismo tiempo se presionó el botón del movimiento a la deracha
          if(P1_rightState == LOW){
            // Si lo hizo entonces también aumenta la coordenada en xp1
            if(xp1 < 280){
            xp1+=5;
            }
          }
          delay(25);
          // En las animaciones 3, 4 y 5 incrementa también la posición en y
          if((yp1 == 3) || (yp1 == 4) ||(yp1 == 5)){
            LCD_Sprite(xp1, yp1init - yp1*2, 40, 77, Scorpion_Jumping, 7, yp1, 0, 0);
            }
          else{
            LCD_Sprite(xp1, yp1init, 40, 77, Scorpion_Jumping, 7, yp1, 0, 0);
            }
          // Dibuja la línea que borra el rastro del jugador
          for(int i = 1; i<=5; i++){
            V_line(xp1-i, yp1init - 5, 82, Backcolor);
            }  
          }
        }
      else if(P1_lr){
        for (int yp1 = 1; yp1 <= 7; yp1++){ 
          // Revisa si se presionaba el botón del movimiento a la izquierda
          if(P1_leftState == LOW){
            if(xp1 > 0){
            // Si lo hacía disminuye la posición en x
            xp1-=5;
            }
          }
          delay(25);
          // Para animaciones 3, 4 y 5 se sube la posición en y
          if((yp1 == 3) || (yp1 == 4) ||(yp1 == 5)){
            LCD_Sprite(xp1, yp1init - yp1*2, 40, 77, Scorpion_Jumping, 7, yp1, 1, 0);
            }
          else{
            LCD_Sprite(xp1, yp1init, 40, 77, Scorpion_Jumping, 7, yp1, 1, 0);
            }
          // Dibuja la línea que borra el rastro del jugador
          for (int i = 39; i <= 43; i++){
            V_line(xp1+i, yp1init - 5, 82, Backcolor);
            }
        }
      }
    }
  
//  if(P1_downState == LOW){
//    // Revisa si el jugador se dirigía a la izquierda o derecha
//    if (!P1_lr){
//        delay(15);
//        // En las animaciones 3, 4 y 5 incrementa también la posición en y
//        LCD_Sprite(xp1, yp1init, 40, 77, Scorpion_Jumping, 7, 1, 0, 0);
//        // Dibuja la línea que borra el rastro del jugador
//        V_line(xp1-1, yp1init, 77, Backcolor);
//        V_line(xp1+40, yp1init, 77, Backcolor);   
//        }
//      }


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
    Serial.println("No se pudo abrir la imagen");
    myFile.close();
      } 
  }

void checkbuttonP1(void){
  P1serial = Serial2.read();
  if(P1serial == 1){
    P1_leftState = LOW;
    }
  if(P1serial == 2){
    P1_upState = LOW;
    }
  if(P1serial == 3){
    P1_rightState = LOW;
    }
  if(P1serial == 4){
    
    }
  if(P1serial == 6){
    P1_leftState = HIGH;
    P1_rightState = HIGH;
    P1_upState = HIGH;
    P1_downState = HIGH;
    }
  }

void portada(){
  //se realiza la animación de la pantalla. 
  
  
  //delay(200);
}

void PICscorpion(){
  Rect(86, 96, 50, 50, 0x075b);
}

void PICsubzero(){
  Rect(196, 96, 50, 50, 0x075b);
}

////////////////////////////////////////////////////////////////
//Verificación de posición y movimiento de personajes
/////////////////////////////////////////////////////////////////
void verifposP1(){
  if (posP1 == 1){ //pos SCORPION
      Rect(196, 96, 50, 50, 0x0000);
      PICscorpion();
    }
  else if (posP1 == 2){ //pos SUBZERO
    Rect(86, 96, 50, 50, 0x0000);
    PICsubzero();
    }
  else if (posP1 > 2){
    posP1 = 1;
  }
  else if (posP1 < 1){
    posP1 = 2;
  }
}

void eleccionP1(){
  
  checkbuttonP1();
  if (P1_leftState == LOW){
    posP1--;
    delay(100);
  }
  else if (P1_rightState == LOW){
    posP1++;
    delay(100);
  }
  else if (P1_upState == LOW){
    P1done = 1;
  }
}


//////////////////
void verifposP2(){
  
  if (posP1 == 1){ //pos SCORPION
      //Rect(196, 96, 50, 50, 0x0000);
    LCD_Bitmap(90, 100, 44, 44, scorpionpic);
    LCD_Bitmap(200, 100, 44, 44, subzeropic);
    //PICscorpion();
    int eleccionP1 = 1;
    int eleccionP2 = 2;
    }
  else if (posP1 == 2){ //pos SUBZERO
        Rect(196, 96, 50, 50, 0x0000);
        Rect(86, 96, 50, 50, 0x0000);
        LCD_Bitmap(200, 100, 44, 44, scorpionpic);
        LCD_Bitmap(90, 100, 44, 44, subzeropic);
        int eleccionP1 = 2;
        int eleccionP2 = 1;
    }
}

//void saveImage(char doc[]){
//  myFile = SD.open(doc, FILE_READ);
//  unsigned char data[76800];
//  int bytesRead = myFile.read(data, 76800);
//  
//  //const unsigned char* dataStored = data;
//  //PROGMEM const unsigned char* dataPointer = dataStored;
//  
//  LCD_Bitmap(0, 0, 320, 210, data);
////  if(myFile){
////    while(myFile.available()){
////      Image[poschar] = myFile.read();
////      Serial.write(Image[poschar]);
////      //Serial.println(myFile.read());
////      poschar++;
////    }
////    Serial.println("Se guardó el archivo");
////    myFile.close();
////    }
//}
