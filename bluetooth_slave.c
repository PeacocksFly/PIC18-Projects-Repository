/*
 * File:   BTSlave.c
 * Author: Quentin
 *
 * Created on 03 June 2019, 19:59
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define RS LATEbits.LATE0
#define RW LATEbits.LATE1
#define EN LATEbits.LATE2

void LCDCommand(uint8_t cmd);
void LCDData(uint8_t cmd);
void writeToLCD(void* p, uint8_t pos);

char value __at(0x050);

void main(void) {
    
    TRISD = 0;
    
    TRISEbits.RE0 = 0;
    TRISEbits.RE1 = 0;
    TRISEbits.RE2 = 0;
    
    ANSELC = 0;
    TRISCbits.RC6 = 0;
    TRISCbits.RC7 = 1;
    
    //UART configuration
    RCSTA1bits.SPEN = 1;                 //serial port enabled
    TXSTA1bits.SYNC = 0;                 //EUSART asynchronous mode selected
    TXSTA1bits.TXEN = 1;                 //transmit bit enabled
    TXSTA1bits.BRGH = 0;                 //baud rate low speed (speed not doubled)
    RCSTA1bits.CREN = 1;
    BAUDCON1bits.BRG16 = 0;              //8-bit baud rate generator
    RCSTA1bits.RX9 = 0;                  //9 bits reception disabled
    SPBRG1 = 0x0C;                       //baud rate = 9600
    
    EN = 0;  
    __delay_ms(100);
    LCDCommand(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCDCommand(0x0C);                   //display on, cursor off
    __delay_ms(5);
    
    writeToLCD((uint8_t*)"Frequency", 0x80);

    while(1)
    {       
        while(PIR1bits.RC1IF);
        value = RCREG1;
        
        LCDData(value);
        __delay_ms(15);
        
    }
    
    
    return;
}


void LCDCommand(uint8_t cmd)
{
    LATD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void LCDData(uint8_t cmd)
{
    LATD = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void writeToLCD(void* p, uint8_t pos)
{
     LCDCommand(pos);
     __delay_ms(5);
     while(*(uint8_t*)p)
     {
           LCDData(*(uint8_t*)p++);
           __delay_ms(5);
     }
}

