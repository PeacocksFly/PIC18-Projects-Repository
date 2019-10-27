/*
 * File:   rs232.c
 * Author: Quentin
 *
 * Created on 22 May 2019, 20:21
 */


#include <xc.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

long value;
void Delay_MilliSeconds(int ms);
void __interrupt(low_priority) myADC(void);

void main(void) {
    
    ANSELB = 1;
    TRISB = 1;
    
    
    ANSELC = 0;
    TRISCbits.RC6 = 0;
    TRISCbits.RC7 = 1;
    
   // ADCON0 = 0x01;
    ADCON0bits.ADON = 1;
    ADCON0bits.CHS = 0b01100;
    ADCON1 = 0x00;
    ADCON2 = 0xAF;
    
    PIR1bits.ADIF = 0;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE1bits.ADIE = 1;  
    
    BAUDCON1bits.BRG16 = 0;
    SPBRG1 = 0x0C;
    TXSTA1bits.TX9 = 0;
    TXSTA1bits.TXEN = 1;
    TXSTA1bits.SYNC = 0;
    TXSTA1bits.BRGH = 0;
    RCSTA1bits.SPEN = 1;
    
    ADCON0bits.GO = 1;
        
    while(1)
    {      
    }
      
    return;
}


void __interrupt(low_priority) myADC(void)
{
    if(PIR1bits.ADIF == 1)
    {
      value = (ADRESH * 256 + ADRESL) * 500 / 1024; 
     //   value = (ADRESL + (ADRESH * 256)) ;
     //   value = ((value * 500) / 1024);

       char temp = value;
       char divider = 100;
       char division = 0;
       char modulo = 0;
       do
       {          
           division = temp/divider;
           modulo = temp%divider;
           temp = modulo;
           divider = divider/10;
           while(PIR1bits.TX1IF == 0){}
           TXREG1 = 0x30 | division;
       }while(divider>0);
       
       while(PIR1bits.TX1IF == 0){}
       TXREG1 = 0x20;
           
       Delay_MilliSeconds(1000);
       
       PIR1bits.ADIF = 0;
       ADCON0bits.GO = 1;
    }

}

void Delay_MilliSeconds(int ms)
{
    for(int i = 0; i< ms; i++)
    {
        __delay_ms(1);
    }   
}