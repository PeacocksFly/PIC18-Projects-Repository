/*
 * File:   4diginterrupt.c
 * Author: Quentin
 *
 * Created on 01 May 2019, 10:46
 */


#include <xc.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000


void __interrupt(low_priority) myTimer(void);

uint8_t digitIndex = 0;                        //global variable to track current digit being refreshed
uint8_t display[] = {0,0,0,0};                 //global array to keep track of 7 segments value
const uint8_t led[] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F}; 

void main(void) {
      
    TRISC = 0;                                 //PORTC as an output
    TRISD = 0;                                 //PORTD as an output
    TRISBbits.RB0 = 1;                         //RB0 as an input
    ANSELBbits.ANSB0 = 0;                      //digital input enabled on RB0
         
    //timer 0 set up
    T0CONbits.T08BIT = 1;                       //8-bit timer 
    T0CONbits.T0CS = 0;                         //counter option disabled
    T0CONbits.PSA = 0;                          //prescaler assigned
    T0CONbits.T0PS = 0b101;                     //prescaler = 64
    TMR0L = 0x64;                               //5 milliseconds delay btw refreshing adjacent digit

    //INT0 and timer0 interrupt set up
    INTCONbits.GIE = 1;                         //global interrupt enabled
    INTCONbits.INT0IE = 1;                      //INT0 interrupt enabled
    INTCONbits.TMR0IE = 1;                      //TMR0 interrupt enabled
    INTCONbits.INT0IF = 0;                      //INT0 flag interrupt reset
    INTCONbits.TMR0IF = 0;                      //TMR0 flag interrupt reset
    INTCON2bits.INTEDG0 = 1;                    //interrupt INT0 on rising edge
           
    LATC = led[display[digitIndex]];            //initial value on PORT C
    LATD = 0x00;                                //initial value on PORT D
    
    T0CONbits.TMR0ON = 1;                       //start timer
    
    while(1);   
    
    return;
}


void __interrupt(low_priority) myTimer(void)
{
    if(INTCONbits.TMR0IF)                       //timer interrupt--> adjacent digit to be refreshed
    {
        T0CONbits.TMR0ON = 0;
                  
        if(LATD)
        {
           digitIndex = (digitIndex + 1)%4;        
           LATD = 0x00;           
           LATC = led[display[digitIndex]];
        }
        else
        {
           LATD = (0x01 << digitIndex);
        }
           
        TMR0L = 0x64;
        INTCONbits.TMR0IF = 0;
        T0CONbits.TMR0ON = 1;
    }
    
    
    
    if(INTCONbits.INT0IF)                       //RB0 interrupt--> counter needs to be incremented
    {
        uint8_t i = 0;
        
        display[0]++;        
        while(display[i]==10)
        {
           display[i++]=0;           
           if(i==4)
               break; 
           display[i]++;
        }
        
        INTCONbits.INT0IF = 0;
    }          
}

