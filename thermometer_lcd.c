/*
 * File:   thermometer_lcd.c
 * Author: Quentin
 *
 * Created on 17 October 2019, 18:41
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define RS LATDbits.LATD0
#define RW LATDbits.LATD1
#define EN LATDbits.LATD2

void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t cmd);
void WriteToLCD(uint8_t* p, uint8_t pos);
uint8_t strlen(uint8_t* s);
void reverse(uint8_t* s);

uint32_t result __at(0x050);

void main(void) {
    
   // uint32_t result;                     //ADC result variable
    uint8_t text[16];                    //array for string to send to LCD
    
    ANSELAbits.ANSA0 = 1;                //digital input buffer disabled
    TRISAbits.RA0 = 1;                   //RA0 configured as an input
    TRISC = 0;                           //PORTC configured as an output
    TRISD = 0;                           //PORTD configured as an output
    
    //ADC converter set up
    ADCON0bits.CHS = 0b00000;            //AN0 channel selected
    ADCON0bits.ADON = 1;                 //ADC enabled
    ADCON1bits.PVCFG = 0b00;             //Vref+ connected to Vdd
    ADCON1bits.NVCFG = 0b00;             //Vref- connected to Vss
    ADCON2bits.ADFM = 1;                 //result right justified
    ADCON2bits.ACQT = 0b100;             //acquisition time = 8 TAD
    ADCON2bits.ADCS = 0b011;             //TAD = 1.7 us (FRC)) 
    
    ADCON0bits.GO = 1;                   //ADC conversion starts
         
    EN = 0;  
    __delay_ms(100);
    LCD_Command(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCD_Command(0x0C);                   //display on, cursor off
    __delay_ms(5);
       
    WriteToLCD((uint8_t*)"Temperature", 0x80);
    
    while(1)
    {
        while(ADCON0bits.nDONE);
        result = ((ADRESL | (uint32_t)ADRESH << 8) * 5000) >> 10;   
                                                //result express in mV as 10mV = 1deg on lm35 
        uint8_t* p = text;
        
        *p++ = 0x64;                            //letter d in ascii
        *p++ = 0x20;                            //space in ascii
        *p++ = 0x30 |(uint8_t)(result%10);      //one digit after comma
        result/= 10;
        *p++ = 0x2E;                            //comma in ascii 
        do{                                     //digits before comma
           *p++ = 0x30 |(uint8_t)(result%10);
        }while((result/=10) > 0);
        *p = '\0';    
        
        reverse(text);                          //text reversed before display
        WriteToLCD(text, 0xC0);                 //write to lcd

        ADCON0bits.GO = 1;
    }
    
    return;
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

uint8_t strlen(uint8_t* s)
{
    uint8_t i = 0;    
    while (*s++ != '\0')
        i++;    
    return i;
}

void reverse(uint8_t* s)
{
    uint8_t c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) 
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }   
}