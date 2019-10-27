/*
 * File:   reaction.c
 * Author: Quentin
 *
 * Created on 09 May 2019, 17:37
 */


#include <xc.h>
#include <stdlib.h>
#include "math.h"
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define RS PORTDbits.RD0
#define RW PORTDbits.RD1
#define EN PORTDbits.RD2

void Delay_MilliSeconds(int ms);
void LCDCommand(unsigned char cmd);
void LCDData(unsigned char cmd);
void __interrupt(high_priority) myTimer(void);

unsigned long time;
unsigned int temp;
unsigned int modulo;

void main(void) {
    
    ANSELC = 0;
    ANSELD = 0;
    TRISC = 0;
    TRISD = 0;
    
    ANSELB = 0;
    TRISB = 1;
    
    LCDCommand(0x0C);
    Delay_MilliSeconds(250); 
    EN = 0;
    
    INTCONbits.GIE = 1;
    INTCONbits.INT0IE = 0;
    INTCONbits.TMR0IE = 0;
    
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.T0SE = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS0 = 1;
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS2 = 1;
    
    T0CONbits.TMR0ON = 0;
    INTCON2bits.INTEDG0 = 1;
    
    while(1)
    {
        if(T0CONbits.TMR0ON == 0)
        {      
           INTCONbits.INT0IF = 0;
           INTCONbits.INT0IE = 1;
           PORTDbits.RD7 = 0;
           Delay_MilliSeconds(3000);
           Delay_MilliSeconds(1 + rand()% 3000);
           PORTDbits.RD7 = 1;
           TMR0  = 0;
           T0CONbits.TMR0ON = 1;
           
        }       
    }
    
            
    return;
}


void __interrupt(high_priority) myTimer(void)
{
    if(INTCONbits.INT0IF==1)
    {
        T0CONbits.TMR0ON = 0;
        char TMRL = TMR0L;
        time = TMR0 ;
        time = (time * 128 / 1000 );
            
        LCDCommand(0x01);
        Delay_MilliSeconds(250);
        LCDCommand(0x80);
        Delay_MilliSeconds(250);

        temp =  time/1000;
        modulo = time%1000;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);

        temp = modulo/100;
        modulo = modulo%100;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);

        temp = modulo/10;
        modulo = modulo%10;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);

        LCDData(modulo | 0x30);
        Delay_MilliSeconds(15);

        INTCONbits.INT0IF=0;
    }
}


void LCDCommand(unsigned char cmd)
{
    PORTC = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    Delay_MilliSeconds(1);
    EN = 0;
  
}

void LCDData(unsigned char cmd)
{
    PORTC = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    Delay_MilliSeconds(1);
    EN = 0;
}

void Delay_MilliSeconds(int ms)
{
    for(int i = 0; i< ms; i++)
    {
        __delay_ms(1);
    }   
}