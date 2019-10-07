/*
 * File:   bipolar_stepper_motor.c
 * Author: Quentin
 *
 * Created on 07 October 2019, 20:31
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000
#define REVS 10
#define CLOCKWISE 1
#define ANTICLOCKWISE -1

uint8_t steps[]={0x01,0x04,0x02,0x08};

void Bipolar_Motor_Turn(uint16_t, uint8_t*, int8_t);

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
    
    Bipolar_Motor_Turn(REVS, &steps[0], CLOCKWISE);  
    __delay_ms(5000);
    Bipolar_Motor_Turn(REVS, &steps[3], ANTICLOCKWISE);
    
    while(1);
    
    return;
}


void Bipolar_Motor_Turn(uint16_t revs, uint8_t* start, int8_t dir)
{
    uint8_t* temp;
    for(uint16_t i=0; i<revs;i++)
    {
        temp = start;
        for(uint8_t j=0; j<4;j++)
        {
            PORTD = *temp;
            temp += dir;
            __delay_ms(250);
        }
    }
}