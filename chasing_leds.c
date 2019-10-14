/*
 * File:   main.c
 * Author: Quentin
 *
 * Created on 06 April 2019, 16:15
 */



#include <xc.h>
#include<stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

uint8_t Led __at(0x050);

void main(void) {
       
    TRISC = 0;                       //port C as output  
    Led = 0x01;

    while(1)
    {      
        LATC =  Led;
        __delay_ms(1000);
        asm("RLNCF 0x050, F");       //circular left rotation without carry
    }
    
    return;
}
