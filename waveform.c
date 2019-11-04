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
#define TMR 143
#define STEPS_SAWTOOTH 10

void __interrupt(low_priority) myTimer(void);
void SendSPI(uint16_t);

bool flag = false;

void main(void) {
    
    TRISC = 0;                         //PORTC as an output
    
    //set up timer
    T0CONbits.T08BIT = 1;              //8-bit timer configured
    T0CONbits.T0CS = 0;                //counter disabled
    T0CONbits.PSA = 0;                 //prescaler assigned
    T0CONbits.T0PS = 0b010;            //prescaler set to 8

    //spi port set up
    SSP1CON1bits.SSPM = 0b0000;        //SPI master mode clk = Fosc/4
    SSP1CON1bits.SSPEN = 1;            //SPI pins enabled
    
    //timer0 interrupt set up
    INTCONbits.GIE = 1;                //global interrupt enabled
    INTCONbits.TMR0IE = 1;             //TMR0 interrupt enabled
    INTCONbits.TMR0IF = 0;             //TMR0 flag interrupt reset
 
    int16_t data = 0;                  //local variable for waveform data
    int8_t dir = 1;                    //increments sign for the triangle
    uint16_t theta = 0;                //angle for the sinewave
    
    TMR0L = TMR;                       //period of sawtooth = 5 ms 
                                       //period of triangle = 10 ms
                                       //period of sinewave = 180 ms
    
    T0CONbits.TMR0ON = 1;              //start timer
    
//Sawtooth
//    while(1)
//    {
//           
//        data = data + (0xFFF / STEPS_SAWTOOTH);  //digital data calculated on 12 bits
//        if(data > 0xFFF )                        
//            data = 0;     
//        SendSPI((uint16_t)data);                 //data sent via spi line
//             
//        while(!flag);                            //wait signal from timer
//        flag = false;                            //flag reset
//    }
    
 //Triangle   
//    while(1)
//    {
//        data += dir * 0x0FFF / STEPS_SAWTOOTH ;
//
//        if(data>0x0FFF || data<0)
//        {
//               dir = -dir;
//               data += dir * 0x0FFF / STEPS_SAWTOOTH;
//        }
//         
//        SendSPI((uint16_t)data);                  
//        
//        while(!flag);
//        flag = false;          
//    }
    
//Sinewave
    while(1)
    { 
        data = 0x07FF * (1 + sin(2*PI*theta/360));
        theta += 10;
        theta %= 360;
        
        SendSPI(data);
        while(!flag);
        flag = false;            
    }
    
    return;
}


void __interrupt(low_priority) myTimer(void)
{
    if(INTCONbits.TMR0IF)                       
    {
        T0CONbits.TMR0ON = 0;                 //stop timer
        flag = true;                          //background process informed
                                              //that waveform can be updated
        TMR0L = TMR;                          //timer reset
        INTCONbits.TMR0IF = 0;                //timer flag lowered
        T0CONbits.TMR0ON = 1;                 //restart timer        
    }

}

void SendSPI(uint16_t data)
{  
    LATCbits.LATC0 = 0;                       //CS line of MCP4921 needs an active-low o enable serial clk
    PIR1bits.SSP1IF = 0;                      //spi flag reset
    SSP1BUF = 0x30 | (uint8_t)(data >> 8);    //first nibble are configuration bits + higher nibble of 12 data bits 
    while(!PIR1bits.SSP1IF);
    PIR1bits.SSP1IF = 0;
    SSP1BUF = (uint8_t)data;                  //8 LSB of the 12 data bits
    while(!PIR1bits.SSP1IF);
    LATCbits.LATC0 = 1;                       //CS line pulled up
}