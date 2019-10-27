/*
 * File:   BTSlave.c
 * Author: Quentin
 *
 * Created on 03 June 2019, 19:59
 */


#include <xc.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define RS PORTEbits.RE0
#define RW PORTEbits.RE1
#define EN PORTEbits.RE2

void Delay_MilliSeconds(int ms);
void LCDCommand(unsigned char cmd);
void LCDData(unsigned char cmd);

char value __at(0x050);

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
    
    ANSELE = 0;
    TRISEbits.RE0 = 0;
    TRISEbits.RE1 = 0;
    TRISEbits.RE2 = 0;
    
    ANSELC = 0;
    TRISCbits.RC6 = 0;
    TRISCbits.RC7 = 1;
    
    TXSTA1bits.SYNC = 0;
    TXSTA1bits.TXEN = 1;
    TXSTA1bits.BRGH = 0;
    TXSTA1bits.TX9 = 0;
    
    RCSTA1bits.CREN = 1;
    RCSTA1bits.SPEN = 1;
    RCSTA1bits.RX9 = 0;
    
    SPBRG1 = 0x0C;
    
    LCDCommand(0x0C);
    Delay_MilliSeconds(250); 
    EN = 0;
    
    
    LCDCommand(0x01);
    Delay_MilliSeconds(250);
    LCDCommand(0x80);
    Delay_MilliSeconds(250);

    while(1)
    {
        
        while(PIR1bits.RC1IF == 0);
        value = RCREG1;
        
        
        
        LCDData(value);
        Delay_MilliSeconds(15);
        
    }
    
    
    return;
}


void LCDCommand(unsigned char cmd)
{
    PORTD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    Delay_MilliSeconds(1);
    EN = 0;
}

void LCDData(unsigned char cmd)
{
    PORTD = cmd;
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
