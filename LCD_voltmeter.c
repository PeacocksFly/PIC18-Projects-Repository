/*
 * File:   LCDvoltmeter.c
 * Author: Quentin
 *
 * Created on 23 April 2019, 20:36
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

void main(void) {
    
    uint16_t result;                     //ADC result variable
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
       
    WriteToLCD((uint8_t*)"Voltmeter", 0x80);
                       
    while(1)
    {
        while(ADCON0bits.nDONE);
        result = ((ADRESL | (uint16_t)ADRESH << 8) * 50) >> 10;
        
        uint8_t* p = text;
        *p++ = 0x30 |(uint8_t)(result/10); //one digit before comma
        *p++ = 0x2E;
        *p++ = 0x30 |(uint8_t)(result%10); //one digit after comma
        *p++ = 0x20;
        *p = 0x56;
        WriteToLCD((uint8_t*)text, 0xC0);  //display result

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
