/*
 * File:   waveform.c
 * Author: Quentin
 *
 * Created on 02 May 2019, 14:11
 */


#include <xc.h>
#include <stdint.h>
#include "math.h"
#include <stdbool.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

#define _XTAL_FREQ 8000000
#define PI 3.14159265358979323846

void __interrupt(low_priority) myTimer(void);
void Send(uint16_t);

uint16_t data = 0;
uint8_t flag = 0;

void main(void) {
    
    TRISC = 0;                         //PORTC as an output
    
    //set up timer
    uint8_t TMRL;
    T0CONbits.T08BIT = 1;              //8-bit timer configured
    T0CONbits.T0CS = 0;                //counter disabled
    T0CONbits.PSA = 0;                 //prescaler assigned
    T0CONbits.T0PS = 0b010;            //prescler set to 8

    //spi port set up
    SSP1CON1bits.SSPM = 0b0000;        //SPI master mode clk = Fosc/4
    SSP1CON1bits.SSPEN = 1;            //SPI pins enabled
    
    //INT0 and timer0 interrupt set up
    INTCONbits.GIE = 1;                         //global interrupt enabled
    INTCONbits.INT0IE = 1;                      //INT0 interrupt enabled
    INTCONbits.TMR0IE = 1;                      //TMR0 interrupt enabled
    INTCONbits.INT0IF = 0;                      //INT0 flag interrupt reset
    INTCONbits.TMR0IF = 0;                      //TMR0 flag interrupt reset
    INTCON2bits.INTEDG0 = 1;                    //interrupt INT0 on rising edge


    char lowerData = 0x00;
    char upperData = 0x00;
    
    int theta = 0;
    
    bool up = true;
    TMRL = 131;                        //
    TMR0L = TMRL;
    T0CONbits.TMR0ON = 1;
    
//Sawtooth
//    while(1)
//    {
//        while(!flag);
//        
//        
//        data = data + (0xFFF / 10);
//        if(data > 0xFFF ){data = 0;}
//        
//        Send(data);
//        
//        flag = 0;
//    }
    
 //Triangle   
//    while(1)
//    {
//
//
//        while(!flag);
//        
//        if(up==true)
//        {
//            data = data + 409;
//            if(data> 4095)
//            {
//                data = data - 409;
//                up=false;
//            }
//        }
//        else
//        {
//            data = data - 409;
//            if(data== 0x0000){up=true;}
//        }
//        
//        Send(data);
//        flag = 0;
//
//            
//    }
    
//Sinewave
    while(1)
    {

        data = (0x07FF * sin(2*PI*theta/360)) + 0x7FF;       
        if(theta == 360){theta = 0;}
        
         while(!flag);
         
        Send(data);
        flag = 0;
        
        theta = theta + 5;
            
    }
//    
//

    
    
    return;
}


void __interrupt(low_priority) myTimer(void)
{
    if(INTCONbits.TMR0IF)                       
    {
        T0CONbits.TMR0ON = 0;
             
        flag = 1;
          
        TMR0L = 131;
        INTCONbits.TMR0IF = 0;
        T0CONbits.TMR0ON = 1;
        
    }

}

void Send(uint16_t data)
{
    PIR1bits.SSP1IF = 0;
    LATCbits.LATC0 = 0;
    SSP1BUF = 0x30 | (uint8_t)(data >> 8)  ;
    while(!PIR1bits.SSP1IF);
    SSP1BUF = (uint8_t)data;
    PIR1bits.SSP1IF = 0;
    while(!PIR1bits.SSP1IF);
    LATCbits.LATC0 = 1;
  
}