/*
 * File:   freqcounter.c
 * Author: Quentin
 *
 * Created on 07 May 2019, 20:31
 */


#include <xc.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP, T3CMX = PORTB5

#define _XTAL_FREQ 8000000
#define RS LATDbits.LATD0
#define RW LATDbits.LATD1
#define EN LATDbits.LATD2

void WriteToLCD(uint8_t* p, uint8_t pos);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t cmd);
void __interrupt(low_priority) myTimer(void);
uint8_t strlen(uint8_t* s);
void reverse(uint8_t* s);
uint8_t* DecimalToASCII(uint8_t* ascii, uint16_t dec);


uint8_t text[16];                      //array for string to send to LCD

void main(void) {
     
    TRISC = 0;                         //Port C and D as output
    TRISD = 0;
    
    TRISB = 1;                         //Port B as input and digital enabled
    ANSELB = 0;
      
    
    INTCONbits.GIE = 1;                //general interrupt enabled
    INTCONbits.TMR0IE = 1;             //tmr0 interrupt enabled

    T0CONbits.T08BIT = 0;              //16-bit timer configured
    T0CONbits.T0CS = 0;                //counter disabled
    T0CONbits.PSA = 0;                 //prescaler assigned
    T0CONbits.T0PS = 0b110;            //prescaler set to 16

    
    T3CONbits.T3SOSCEN = 0;            //secondary oscillator disabled
    T3CONbits.TMR3CS = 0b10;           //T3CKI pin enabled
    T3CONbits.T3CKPS1 = 0b00;          //prescaler = 0
 

    EN = 0;  
    __delay_ms(100);
    LCD_Command(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCD_Command(0x0C);                   //display on, cursor off
    __delay_ms(5);
    
    WriteToLCD((uint8_t*)"Frequency", 0x80);
    
    TMR3=0;
    TMR0=0xC2F7;
   
    T3CONbits.TMR3ON = 1;
    T0CONbits.TMR0ON = 1;
    while(1);
    
    return;
}

void __interrupt(low_priority) myTimer(void)
{

     if(INTCONbits.TMR0IF==1)                   //long interrupt latency but no impact as we do
                                                //only wish to measure the frequency without deadline
     { 
        T3CONbits.TMR3ON = 0;                   //stop counter
        T0CONbits.TMR0ON = 0;                   //stop timer
        
        uint16_t time = TMR3 ;                  //tmr3 is a 16-bit counter so limited to 65535 Hz
                 
        uint8_t* p = text;     
        *p++ = 0x7A;                            //letter z in ascii
        *p++ = 0x48;                            //letter H in ascii
        *p++ = 0x20;                            //space in ascii
         p = DecimalToASCII(p,time);            //decimal to ascii
        *p = '\0';                              //null character        
        reverse(text);                          //text reversed before display
        WriteToLCD(text, 0xC0);                 //write to lcd
             
        TMR3 = 0;                               //counter reset
        TMR0=0xC2F7;                            //timer reset      

        INTCONbits.TMR0IF=0;
        T3CONbits.TMR3ON = 1;                   //start timer
        T0CONbits.TMR0ON = 1;                   //start counter

     }
}


uint8_t* DecimalToASCII(uint8_t* ascii, uint16_t dec)
{
        do{                                     
           *ascii++ = 0x30 |(uint8_t)(dec%10);
        }while((dec/=10) > 0);
        
        return ascii;
}

void WriteToLCD(uint8_t* p, uint8_t pos)
{
     LCD_Command(pos);
     __delay_ms(5);
     while(*p)
     {
           LCD_Data((uint8_t)*p++);
           __delay_ms(5);
     }
}

void LCD_Command(uint8_t cmd)
{
    LATC = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void LCD_Data(uint8_t cmd)
{
    LATC = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

uint8_t strlen(uint8_t* s)               //calculate length of an array terminated by null character    
{
    uint8_t i = 0;    
    while (*s++ != '\0')
        i++;    
    return i;
}

void reverse(uint8_t* s)                 //reverse an array of terminated by null character 
{
    uint8_t c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) 
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }   
}