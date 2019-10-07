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
#define REVS 20
#define CLOCKWISE -1
#define ANTICLOCKWISE 1

uint8_t steps[]={0x01,0x02,0x04,0x08};

void Motor_Turn(uint16_t, uint8_t*, int8_t);

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
      
    Motor_Turn(REVS, &steps[3], CLOCKWISE);  
    __delay_ms(5000);
    Motor_Turn(REVS/4, &steps[0], ANTICLOCKWISE);
    __delay_ms(3000);
    Motor_Turn(REVS/2, &steps[3], CLOCKWISE);
    __delay_ms(1000);
    
    while(1);
    return;
}


void Motor_Turn(uint16_t revs, uint8_t* start, int8_t dir)
{
    uint8_t* temp;
    for(uint16_t i=0; i<revs;i++)
    {
        temp = start;
        for(uint8_t j=0; j<4;j++)
        {
            PORTD = *temp;
            temp += dir;
            __delay_ms(100);
        }
    }
}