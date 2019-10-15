/*
 * File:   led_dice.c
 * Author: Quentin
 *
 * Created on 14 October 2019, 20:21
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

void __interrupt(low_priority) myLoIsr(void);
int rand(void) ;
void srand(unsigned int);

uint8_t dice_output[]={0x08, 0x14, 0x1C, 0x63, 0x6B, 0x77};
uint32_t next = 1;

void main(void) {
    
    TRISC = 0;
    TRISBbits.RB0 = 1;
    ANSELBbits.ANSB0 = 0;
    
    //INT0 and INT1  set up
    INTCONbits.GIE = 1;           //global interrupt enabled
    INTCONbits.INT0IE = 1;        //interrupt enabled for INT0
    INTCON2bits.INTEDG0 = 0;      //interrupt INT0 on falling edge
    INTCON2bits.nRBPU = 0;        //use of weak pull-up features on PORT B
    WPUB = 0x01;
   
    srand(5);
    while(1){
    }

    return;
}


void __interrupt(low_priority) myLoIsr(void)
{
    if(INTCONbits.INT0IF)
    {
       LATC = dice_output[rand() / (RAND_MAX / 6 + 1)];      
       INTCONbits.INT0IF = 0;
    }
}

int rand(void)  // RAND_MAX equal to 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed)
{
    next = seed;
}