/*
 * File:   freqcounter.c
 * Author: Quentin
 *
 * Created on 07 May 2019, 20:31
 */


#include <xc.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP, T3CMX = PORTB5
#define _XTAL_FREQ 8000000

#define RS PORTDbits.RD0
#define RW PORTDbits.RD1
#define EN PORTDbits.RD2

void Delay_MilliSeconds(int ms);
void LCDCommand(unsigned char cmd);
void LCDData(unsigned char cmd);
void __interrupt(low_priority) myTimer(void);

unsigned int test __at(0x050);

unsigned int time;
int temp;
int modulo;

void main(void) {
    
    ANSELC = 0;
    ANSELD = 0;
    ANSELA = 0;
    
    TRISA = 1;
    TRISC = 0;
    TRISD = 0;
    
    ANSELB = 0;
    TRISB = 1;  
    
    INTCONbits.GIE = 1;
    INTCONbits.TMR0IE = 1;
    INTCONbits.RBIE = 0;
    
//    INTCONbits.INT0IE = 0;
    
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS = 0;
    T0CONbits.T0SE = 0;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS0 = 0;
    T0CONbits.T0PS1 = 1;
    T0CONbits.T0PS2 = 1;
    
    T3CONbits.TMR3CS1 = 1;
    T3CONbits.TMR3CS0 = 0;
    T3CONbits.T3SOSCEN = 0;
    T3CONbits.T3CKPS1 = 0;
    T3CONbits.T3CKPS0 = 0;
//  T3CONbits.NOT_T3SYNC = 0;
    

//    T3CONbits.T3RD16 = 1;
//    T3CONbits.nT3SYNC = 1;
//    T3GCONbits.TMR3GE = 1;

    
    LCDCommand(0x0C);
    Delay_MilliSeconds(250); 
    EN = 0;
           
     
    TMR3L=0;
    TMR3H=0;
    TMR0L=0xF7;
    TMR0H=0xC2;
   
    
    T3CONbits.TMR3ON = 1;
    T0CONbits.TMR0ON = 1;

  //  INTCONbits.INT0IE = 1;

    
    while(1)
    {
    }
    
    return;
}

void __interrupt(low_priority) myTimer(void)
{

//Method 2
//    if(INTCONbits.INT0IF==1)
//    {
//        if(T0CONbits.TMR0ON==0)
//        {
//
//            
//            T0CONbits.TMR0ON  = 1;
//            INTCONbits.INT0IF = 0;
//        }
//        else
//        {
//            T0CONbits.TMR0ON = 0;
//            INTCONbits.INT0IE=0;
//            
//            char TMRL = TMR0L;
//            time = TMR0 ;
//
//            LCDCommand(0x01);
//            Delay_MilliSeconds(250);
//            LCDCommand(0x80);
//            Delay_MilliSeconds(250);
//
//            temp =  time/1000;
//            modulo = time%1000;
//            LCDData(temp | 0x30);
//            Delay_MilliSeconds(15);
//
//            temp = modulo/100;
//            modulo = modulo%100;
//            LCDData(temp | 0x30);
//            Delay_MilliSeconds(15);
//
//            temp = modulo/10;
//            modulo = modulo%10;
//            LCDData(temp | 0x30);
//            Delay_MilliSeconds(15);
//
//            LCDData(modulo | 0x30);
//            Delay_MilliSeconds(15);
//
//            Delay_MilliSeconds(1000);
//            
//            
//            TMR0 = 0;
//            
//            INTCONbits.INT0IF=0;
//            INTCONbits.INT0IE=1;
//            
//            
//
//        
//        }
//    }
  
//Method 1
     if(INTCONbits.TMR0IF==1)
     {
       
        char TMRL = TMR3L;
        time = TMR3 ;
         
        LCDCommand(0x01);
        Delay_MilliSeconds(250);
        LCDCommand(0x80);
        Delay_MilliSeconds(250);

        temp =  time/10000;
        modulo = time%10000;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);
        
        temp = modulo/1000;
        modulo = modulo%1000;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);

        temp = modulo/100;
        modulo = modulo%100;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);

        temp = modulo/10;
        modulo = modulo%10;
        LCDData(temp | 0x30);
        Delay_MilliSeconds(15);

        LCDData(modulo | 0x30);
        Delay_MilliSeconds(15);
         
        
        TMR3 = 0;
   //     TMR3H = 0;
        TMR0=0xC2F7;
   //     TMR0H=0x;

        INTCONbits.TMR0IF=0;

     }
}

void LCDCommand(unsigned char cmd)
{
    PORTC = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    Delay_MilliSeconds(1);
    EN = 0;
  
}

void LCDData(unsigned char cmd)
{
    PORTC = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    Delay_MilliSeconds(1);
    EN = 0;
}

void Delay_MilliSeconds(int ms)
{
    for(int i = 0; i< ms; i++)
    {
        __delay_ms(1);
    }   
}