#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

#define _XTAL_FREQ 8000000
#define EN1 PORTEbits.RE1
#define EN2 PORTEbits.RE2

const uint8_t Led[] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
volatile uint8_t i = 0;  
volatile uint8_t j = 0;

void main(void) {
    
    TRISE = 0;
    TRISC = 0;
    
    EN1 = 0;
    EN2 = 0;

    //timer interrupt set up
    INTCONbits.GIE = 1;              //global interrupt enabled
    INTCONbits.TMR0IE = 1;           //timer0 interrupt enabled            
    INTCONbits.TMR0IF = 0;           //timer0 flag reset (extra caution!)        
      
    //timer delay set up   
    T0CONbits.T08BIT = 0;            //16-bit timer
    T0CONbits.T0CS = 0;              //counter transition disabled                  
    T0CONbits.T0PS = 0b110;          //prescaler = 128
    T0CONbits.PSA = 0;               //prescaler assigned 
    
    TMR0 = 0xC2F7;                   //timer fed every 64 us = 1/(8MHz/4/128)
                                     //delay of 1s = (65536 - (1000000/64))
                                     //delay = 49911 = 0xC2F7
    T0CONbits.TMR0ON = 1;            //timer0 start
    
    while(1)
    {                    
        LATC = Led[j];               //decimal digit
        EN1 = 1;
        __delay_ms(5);
        EN1 = 0;
        
        LATC = Led[i];               //unit digit
        EN2 = 1;
        __delay_ms(5);
        EN2 = 0;       
    }
        
    return;
}


void __interrupt(low_priority) myLoIsr(void)
{
    if(INTCONbits.TMR0IF)
    {
        T0CONbits.TMR0ON = 0;
        j = i==9 ? (j==5 ? 0 : ++j) : j;
        i = i==9 ? 0 : ++i;
        
        TMR0 = 0xC2F7;
        INTCONbits.TMR0IF = 0;
        T0CONbits.TMR0ON = 1;
    }   
}


