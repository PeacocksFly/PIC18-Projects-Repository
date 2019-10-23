/*
 * File:   calculator.c
 * Author: Quentin
 *
 * Created on 01 May 2019, 20:24
 */


#include <xc.h>
#include <stdint.h>
#include <math.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define RS LATBbits.LATB0
#define EN LATBbits.LATB1
#define KEYPAD_ERR 16

void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t cmd);
void WriteToLCD(uint8_t* p, uint8_t pos);
uint8_t Strlen(uint8_t* s);
void Reverse(uint8_t* s);
uint8_t KeySelection();
void InputNumber(uint16_t* num1);
void Operation(int32_t* result, uint16_t* num1, uint16_t* num2);

uint8_t keys[4][4]={{1, 2, 3, 4},                       //keypad matrix
                   {5, 6, 7, 8},
                   {9, 0, 10, 11},
                   {12, 13, 14, 15}};        

void main(void) {
        
    uint16_t number1;                                   
    uint16_t number2;
    int32_t result;    
    uint8_t text[16];
    uint8_t *p;
    
    ANSELD = 0xF0;                                    
    TRISD = 0x0F;                                    //low nibble as an input to detect key hit
    TRISC = 0;                                       
    TRISB = 0;
      
    EN = 0;  
    __delay_ms(100);
    LCD_Command(0x38);                                //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCD_Command(0x0C);                                //display on, cursor off
    __delay_ms(5);

    WriteToLCD((uint8_t*)"Calculator", 0x80); 
    
    do
    {
        number1 = 0;
        number2 = 0;
        p = text;
        
        __delay_ms(3000);                            //delay before screen refresh
        LCD_Command(0x01);                           //refresh screen

        WriteToLCD((uint8_t*)"Number1 = ", 0x80);    //operand 1 request
        InputNumber(&number1);

        WriteToLCD((uint8_t*)"Number2 = ", 0xC0);    //operand 2 request
        InputNumber(&number2);

        WriteToLCD((uint8_t*)"Operation: ", 0x90);   //operation type request
        Operation(&result, &number1, &number2);

        WriteToLCD((uint8_t*)"Out = ", 0xD0);        //result display
        if(result==0x45)
        {       
            WriteToLCD((uint8_t*)"Error", 0xD6);
        }
        else
        {
            int8_t sign = (result < 0) ? -1 : 1;     //check sign
            result *= (int32_t)sign;              
            do
            {                                        
                *p++ = 0x30 |(uint8_t)(result%10);
            }while((result/=(int32_t)10) > 0);
            
            if (sign==-1)
                *p++ = 0x2D;
            *p = '\0';                                     
            Reverse(text);                          //text reversed before display
            WriteToLCD(text, 0xD6);                 //write to lcd     
        }      
    }while(1);         

    return;
}


void InputNumber(uint16_t* num)
{
    uint8_t counter = 0;
    uint8_t key;
            
    do
    {
        key = KeySelection();                     //only positive operand allowed (potential improvement)
        if(key <=9 && counter < 4)                //limitation to 4 figures to avoid overflow in result
        {     
            LCD_Data(key | 0x30);                 //display the digit using ascii
            *num = (*num * 10) + key;             //update the operand total value
            counter++;
        }       
        __delay_ms(250);       
    }while(key != 11 || counter==0);              //at least one digit must be selected and then enter pressed
    
    return;
}

void Operation(int32_t* result, uint16_t* num1, uint16_t* num2)
{
    uint8_t operation;
    
    do{
        operation = KeySelection();       
    }while(operation <12);
       
    switch(operation)                         
    {
        case 12: LCD_Data(0x2B);                  //sum
                 *result = *num1 + *num2;
                 break;
        case 13: LCD_Data(0x2D);                  //difference
                 *result = (int32_t)*num1 - *num2;
                 break;
        case 14: LCD_Data(0x2A);                  //product
                 *result = (int32_t)*num1 * *num2;
                 break;
        case 15: LCD_Data(0x2F);                  //division (division by 0 leads to an error)
                 *result = *num2 == 0? 0x45: *num1 / *num2; 
                 break;
        default: break;
    }
    return;
}


uint8_t KeySelection()
{
    uint8_t selectedKey = KEYPAD_ERR;
    
    do{
        for(uint8_t i=0; i<4;i++)                 //high nibble PORT D changes every loop 
        {                                         //polling on the low nibble to identify key hit
           LATD = 0x10 << i;
           if(PORTDbits.RD0) {selectedKey = keys[0][3-i];}
           if(PORTDbits.RD1) {selectedKey = keys[1][3-i];}
           if(PORTDbits.RD2) {selectedKey = keys[2][3-i];}
           if(PORTDbits.RD3) {selectedKey = keys[3][3-i];}
        }        
    }while(selectedKey==KEYPAD_ERR);
 
    return selectedKey;
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
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void LCD_Data(uint8_t cmd)
{
    LATC = cmd;
    RS = 1;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

uint8_t Strlen(uint8_t* s)               //calculate length of an array terminated by null character    
{
    uint8_t i = 0;    
    while (*s++ != '\0')
        i++;    
    return i;
}

void Reverse(uint8_t* s)                 //reverse an array of terminated by null character 
{
    uint8_t c, i, j;
    for (i = 0, j = Strlen(s)-1; i < j; i++, j--) 
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }   
}