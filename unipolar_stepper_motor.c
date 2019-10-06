/*
 * File:   unipolar_stepper_motor.c
 * Author: Quentin
 *
 * Created on 02 October 2019, 20:59
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000
#define REVS 200

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
          
    for(uint16_t i=0; i<REVS;i++)
    {
        for(uint8_t j=0; j<4;j++)
        {
            PORTD = 0x01 << j;
            __delay_ms(30);
        }
    }
    
    while(1);
    
    return;
}
