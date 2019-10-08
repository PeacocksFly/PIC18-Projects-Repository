/*
 * File:   dc_motor.c
 * Author: Quentin
 *
 * Created on 08 October 2019, 18:52
 */


#include <xc.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

void main(void) {
    
    ANSELD = 0x00;
    TRISD = 0x00;
    
    ANSELB = 0x00;
    TRISB = 0x03;
    
    INTCON2bits.nRBPU = 0;    //use of weak pull-up features on PORT B
    WPUB = 0x03;              //weak pull-up used for RB0 and RB1
 
    while(1)
    {
        if(!PORTBbits.RB0)
        {
            if(PORTBbits.RB1)
               PORTD = 0x09;
            else
               PORTD = 0x06;     
        }
        else
            PORTD = 0x00;
    }
    
    return;
}
