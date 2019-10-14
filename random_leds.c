/*
 * File:   random.c
 * Author: Quentin
 *
 * Created on 08 April 2019, 19:29
 */



#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

void main(void) 
{    
    TRISC = 0;    
    srand(10);                                  //seed for the pseudo-random generator
    
    while(1)
    {
        LATC = (uint8_t)(rand()%255+1);         //random numbers generated between 1 and 255
        __delay_ms(1000);
    }      
    return;
}

