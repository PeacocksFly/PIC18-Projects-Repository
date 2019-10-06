/*
 * File:   complex_control_unipolar_stepper_motor.c
 * Author: Quentin
 *
 * Created on 06 October 2019, 21:19
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000
#define REVS 200

uint8_t steps[]={0x01,0x02,0x04,0x08};

void Motor_Turn(uint16_t revs);

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
      
    Motor_Turn(REVS, );  
    __delay_ms(5000);
    Motor_Turn(REVS/4);
    __delay_ms(3000);
    Motor_Turn(REVS/2);
    __delay_ms(1000);
    
    while(1);
    return;
}


void Motor_Turn(uint16_t revs)
{
    for(uint16_t i=0; i<revs;i++)
    {
        for(uint8_t j=0; j<4;j++)
            for(uint8_t j=3; j>=;j--)
        {
            PORTD = (0x01 << j);
            __delay_ms(50);
        }
    }
}