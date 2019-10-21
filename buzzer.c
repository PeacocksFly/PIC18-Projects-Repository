/*
 * File:   buzzer.c
 * Author: Quentin
 *
 * Created on 25 April 2019, 13:25
 */


#include <xc.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

void __interrupt(low_priority) myLoIsr(void);
void delay_ms(uint16_t time);

uint16_t notesFrequency[] ={262,277,294,311,330,349,370,392,415,440,466,494,524};  //frequency of notes
const uint8_t notesSequence[] ={0,0,2,0,5,4,0,0,2,0,7,5,0,0,12,9,5,4,2,10,10,9,5,7,5};   //sequence to get happy birthday
const uint8_t notesDuration[] ={1,1,2,2,2,3,1,1,2,2,2,3,1,1,2,2,2,2,2,1,1,2,2,2,4};      //notes duration in milliseconds 

uint16_t timer;                      //global variable to store the timer for the current note

void main(void) {
    
    TRISCbits.RC2 = 0;
    
    //set up timer
    INTCONbits.GIE = 1;              //global interrupt enabled
    INTCONbits.TMR0IE = 1;           //timer0 interrupt enabled
    INTCONbits.TMR0IF = 0;           //timer0 flag reset (caution)

    T0CONbits.T08BIT = 0;            //16-bit timer selected
    T0CONbits.T0CS = 0;              //counter option disabled
    T0CONbits.PSA = 0;               //prescaler used
    T0CONbits.T0PS = 0b000;          //prescaler = 2

    for(int i=0; i<13; i++)
        notesFrequency[i] = (65536 - ((50000/notesFrequency[i])*10));     //timer input calculation
   
    for(uint8_t i=0; i<25; i++)
    {
         timer = notesFrequency[notesSequence[i]];         //timer is set with frequency of the note to be played
         TMR0 = timer;                                     //timer is loaded
         T0CONbits.TMR0ON = 1;                             //timer is started
         delay_ms(notesDuration[i]*400);                   //delay corresponding to the note duration
         T0CONbits.TMR0ON = 0;                             //timer is stopped before moving to the next note
    }  
}

void __interrupt(low_priority) myLoIsr(void)
{    
    T0CONbits.TMR0ON = 0;
    LATCbits.LATC2 = ~LATCbits.LATC2;                      
    TMR0 = timer;
    INTCONbits.TMR0IF = 0;
    T0CONbits.TMR0ON = 1;
}


void delay_ms(uint16_t time)
{
    for(uint16_t i=0; i<time; i++)
         __delay_ms(1);
}