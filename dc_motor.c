/*
 * File:   dc_motor.c
 * Author: Quentin
 *
 * Created on 08 October 2019, 18:52
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000
#define CLOCKWISE 1
#define ANTICLOCKWISE -1

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
    
    PORTD = 0x09;
    __delay_ms(5000);
    PORTD = 0x06;
    __delay_ms(5000);
    
    while(1);
    return;
}
