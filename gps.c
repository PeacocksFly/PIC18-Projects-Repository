/*
 * File:   gps.c
 * Author: Quentin
 *
 * Created on 26 May 2019, 10:26
 */


#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000


void LCD_Data(uint8_t cmd);
void LCD_Command(uint8_t cmd);
void WriteToLCD(uint8_t* p, uint8_t pos);
void __interrupt(low_priority) UARTInterrupt(void);

#define RS LATEbits.LATE0
#define RW LATEbits.LATE1
#define EN LATEbits.LATE2

const uint8_t GPRMC[] = "$GPGGA" ;
uint8_t received_byte ;

void main(void) {
    

    TRISD = 0;                          //Port D as an output
    
    ANSELC = 0;                         //digital input enabled
    TRISCbits.RC7 = 1;                  //RC7 as an input
    
    TRISEbits.RE0 = 0;
    TRISEbits.RE1 = 0;
    TRISEbits.RE2 = 0;
    
    //RS232 configuration
    RCSTA1bits.SPEN = 1;                 //serial port enabled
    RCSTA1bits.RX9 = 0;                  //9-bits reception disabled
    RCSTA1bits.CREN = 1;                 //async mode received enabled
    TXSTA1bits.SYNC = 0;                 //EUSART asynchronous mode selected
    BAUDCON1bits.BRG16 = 0;              //8-bit baud rate generator
    SPBRG1 = 0x0C;                       //baud rate = 9600
    TXSTA1bits.BRGH = 0;                 //baud rate low speed (speed not doubled)

    
    INTCONbits.GIE = 1;                  //general interrupt enabled         
    INTCONbits.PEIE = 1;                 //peripheral interrupt enabled
    PIE1bits.RC1IE = 1;                  //RX1 enabled
    
    EN = 0;  
    __delay_ms(100);
    LCD_Command(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCD_Command(0x0C);                   //display on, cursor off
    __delay_ms(100);
    
    WriteToLCD((uint8_t*)"GPS Position", 0x80);
    WriteToLCD((uint8_t*)"UTC:", 0xC0);
    WriteToLCD((uint8_t*)"Lon:", 0x90);
    WriteToLCD((uint8_t*)"Lat:", 0xD0);   

    while(1);
             
    return;
}


void __interrupt(low_priority) UARTInterrupt(void)          //give 9600 bps, one byte received every 1.04 ms
{                                                           //displaying one character takes less around 60 us
                                                            //so that despite the interrupt overhead and the parsing
                                                            //if else sequence it is possible to display characters
                                                            //directly on lcd instead of storing them in an array
                                                            //and then parse them what would have caused missing
                                                            //some characters    
      static uint8_t index = 0;
      static uint8_t comma_counter = 0;
      static uint8_t character_pos = 0;
  
      if(PIR1bits.RC1IF)
      {
          received_byte = RCREG1;

          if(index < 6)                                     //identify the sequence $GPGGA from NMEA frame
          {
              index = GPRMC[index] == received_byte ?  ++index : 0;
          }
          else
          {
              if(received_byte == 0x2C)                     //then parse using the commas
              {
                  ++comma_counter;
                  character_pos = 0;
              }
              
              if(comma_counter==1 &&  character_pos > 0  && character_pos < 9)
              {                                             //first comma is the time
                  WriteToLCD(&received_byte, 0xC3 + character_pos);
                  if(character_pos == 2 || character_pos == 5)
                      WriteToLCD((uint8_t*)":", 0xC3 + ++character_pos);
              }
              else if((comma_counter== 2 &&  character_pos > 0))
              {                                             //second comma the longitude degree and minutes
                  WriteToLCD(&received_byte, 0x93 + character_pos);
                  if(character_pos == 2)
                      WriteToLCD((uint8_t*)"d", 0x93 + ++character_pos);          
              }
              else if((comma_counter== 3 &&  character_pos > 0))
              {                                             //third comma is the longitude north or south
                  WriteToLCD(&received_byte, 0x9F);           
              }
              else if((comma_counter==4 &&  character_pos > 0))
              {                                             //fourth comma is the latitude degree and minutes
                  WriteToLCD(&received_byte, 0xD3 + character_pos);
                  if(character_pos == 3)
                      WriteToLCD((uint8_t*)"d", 0xD3 + ++character_pos);       
              }
              else if((comma_counter== 5 &&  character_pos > 0))
              {                                             //fifth comma is the latitude west or east
                  WriteToLCD(&received_byte, 0xDF);           
              }
              
              character_pos++;
              
              if(comma_counter == 6)                        //reset the sequence
              {
                  comma_counter = 0;
                  index = 0;
              }
          }
      }
}

void WriteToLCD(uint8_t* p, uint8_t pos)
{
     LCD_Command(pos);
     __delay_us(25);
     while(*p)
     {
           LCD_Data((uint8_t)*p++);
           __delay_us(25);
     }
}


void LCD_Command(uint8_t cmd)
{
    LATD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    __delay_us(1);
    EN = 0;
}

void LCD_Data(uint8_t cmd)
{
    LATD = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    __delay_us(1);
    EN = 0;
}

