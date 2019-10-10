/*
 * File:   dc_motor_speed_control.c
 * Author: Quentin
 *
 * Created on 09 October 2019, 21:00
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define INC 1 
#define DEC -1
#define STEP 10                             //duty cycle step
#define PR2_10BITS (PR2 + 1) * 4
#define INCDEC_LEVEL PR2_10BITS / STEP      //10% step inc/dec duty cycle 

void Change_Duty_Cycle(int8_t);

void main(void) {
    
    ANSELC = 0x00;
    TRISCbits.RC2 = 0;
    
    ANSELB = 0x00;
    TRISB = 0x03;
    
    //INT0 and INT1  set up
    INTCONbits.GIE = 1;           //global interrupt enabled
    INTCONbits.INT0IE = 1;        //interrupt enabled for INT0
    INTCON3bits.INT1E = 1;        //interrupt enabled for INT1
    INTCON2bits.INTEDG0 = 0;      //interrupt INT0 on falling edge
    INTCON2bits.INTEDG1 = 0;      //interrupt INT1 on falling edge
    INTCON2bits.nRBPU = 0;        //use of weak pull-up features on PORT B
    WPUB = 0x03;
    
    //PWM set up
    CCP1CON = 0x0C;                //PWM Mode    
    CCPTMRS0bits.C1TSEL = 0b00;    //CCP1 - PWM modes use Timer2   
    PR2 = 0x63;                    //F_pwm = 5kHz --> PR2 = 99 (formula in datasheet PIC18F45K22 p181)   
    CCPR1L = 0x32;                 //initial 50% duty cycle = 50% * ((PR2 + 1) * 4) = 200 --> MSB in CCOR1L
    CCP1CONbits.DC1B = 0b00;       //2 LSB of 200 in DC1B
    T2CONbits.T2CKPS = 0b01;       //Prescaler = 4   
    
    T2CONbits.TMR2ON = 1;          //start timer 2
         
    while(1)
    {
        PIR1bits.TMR2IF = 0;       
        while(PIR1bits.TMR2IF==0); //timer 2 polling
    }
    
    return;
}


void __interrupt(low_priority) myLoIsr(void)
{
    if(INTCONbits.INT0IF)
    {
        Change_Duty_Cycle(INC);
        INTCONbits.INT0IF = 0;
    }
    
    if(INTCON3bits.INT1IF)
    {
        Change_Duty_Cycle(DEC);
        INTCON3bits.INT1IF = 0;
    }
}


void Change_Duty_Cycle(int8_t dir)
{
    int16_t new_pwm = (int16_t)CCPR1L << 2 | ((int16_t)CCP1CON & 0x30) >> 4;     
    new_pwm +=  (int16_t)(dir * INCDEC_LEVEL);

    if(new_pwm < 0 || new_pwm > PR2_10BITS)
        return;
    
    CCPR1L =  (uint8_t)(new_pwm >> 2);   
    CCP1CON = CCP1CON | (uint8_t)((new_pwm & 0x0003) << 4);         
}