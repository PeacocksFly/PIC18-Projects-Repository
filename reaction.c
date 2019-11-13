/*
 * File:   reaction.c
 * Author: Quentin
 *
 * Created on 09 May 2019, 17:37
 */


#include <xc.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

#define _XTAL_FREQ 8000000
#define RS LATDbits.LATD0
#define RW LATDbits.LATD1
#define EN LATDbits.LATD2

void DelayMs(uint16_t del);
void WriteToLCD(uint8_t* p, uint8_t pos);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t cmd);
void __interrupt(high_priority) myTimer(void);
uint8_t strlen(uint8_t* s);
void reverse(uint8_t* s);
uint8_t* DecimalToASCII(uint8_t* ascii, uint16_t dec);
uint16_t randN(void) ;
void srandN(uint16_t);

uint16_t reaction_time;              
uint8_t text[16];                      //array for string to send to LCD
uint32_t next = 1;

void main(void) {
    
    TRISC = 0;                         //Port C and Port D as output
    TRISD = 0;
    
    TRISB = 1;                         //Port B as input
    ANSELB = 0;                        //digital input enabled
  
    INTCONbits.GIE = 1;                //general interrupt enabled
    INTCONbits.INT0IE = 0;             //Interrupt on RB0 enabled
    INTCONbits.TMR0IE = 1;             //tmr0 interrupt enabled
    
    T0CONbits.T08BIT = 0;              //16-bit timer enabled
    T0CONbits.T0CS = 0;                //counter function disabled
    T0CONbits.PSA = 0;                 //prescaler enabled
    T0CONbits.T0PS = 0b111;            //prescaler of 256 (maximum reaction time is 8.4 seconds)
  
    T0CONbits.TMR0ON = 0;              //stop timer
    INTCON2bits.INTEDG0 = 0;           //interrupt on falling edge
    
    srandN(10);
    
    EN = 0;  
    __delay_ms(100);
    LCD_Command(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCD_Command(0x0C);                   //display on, cursor off
    __delay_ms(5);
    
    WriteToLCD((uint8_t*)"Reaction Timer", 0x80);
    
    while(1)
    {
        if(!T0CONbits.TMR0ON)
        {      
                  
           LATDbits.LATD7 = 0;                             //turn off the led    
           DelayMs(2000 + randN()%3000);                   //generate random delay between min 2s and max 5s
           LATDbits.LATD7 = 1;                             //turn on the led           
           T0CONbits.TMR0ON = 1;                           //start the timer
           INTCONbits.INT0IF = 0;                          //necessary to reset flag before enabling the interrupt
                                                           //otherwise any click before the light on will be taken into account
           INTCONbits.INT0IE = 1;                          //enable interrupt only after turning on light
                                                           //to avoid cheating, small overhead of two instruction times
                                                           //that will impact the real reaction time 
        }       
    }
    
            
    return;
}


void __interrupt(high_priority) myTimer(void)     //TMR0 and INT0 are both high priority interrupts
{                                                 //the push button will be served first if both are activated 
                                                  //at the same time
    if(INTCONbits.INT0IF)
    {
        T0CONbits.TMR0ON = 0;                     //stop timer  
        INTCONbits.TMR0IF = 0;                    //ensure that if the timer triggered just 
                                                  //after if statement and before stopping the timer
                                                  //the if below will not be executed
        INTCONbits.INT0IE = 0;                    //disable interrupt
        INTCONbits.INT0IF = 0;                    //lower interrupt flag
        
        reaction_time = (uint16_t)(TMR0 * (uint32_t)128 / 1000) ;
            
        uint8_t* p = text;     
        *p++ = 0x73;                            //letter s in ascii
        *p++ = 0x6D;                            //letter m in ascii
        *p++ = 0x20;                            //space in ascii
         p = DecimalToASCII(p,reaction_time);   //decimal to ascii
        *p = '\0';                              //null character        
        reverse(text);                          //text reversed before display
        WriteToLCD(text, 0xC0);                 //write to lcd
        DelayMs(1000);
        WriteToLCD((uint8_t*)"                ", 0xC0);
                                                //refresh screen
        TMR0  = 0;                              //reset timer           
    }
    
    if(INTCONbits.TMR0IF)
    {      
        T0CONbits.TMR0ON = 0;                   //stop timer
        INTCONbits.INT0IE = 0;                  //disable button interrupt
                                                //so that after serving this interrupt
                                                //even if button is pushed nothing happens
        INTCONbits.TMR0IF = 0;                  //reset timer flag

        WriteToLCD((uint8_t*)"Max time expired", 0xC0);
        DelayMs(2000);
        WriteToLCD((uint8_t*)"                ", 0xC0);
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

uint16_t randN(void)  // RAND_MAX equal to 32767
{
    next = next * 1103515245 + 12345;
    return (uint16_t)(next/65536) % 32768;
}

void srandN(uint16_t seed)
{
    next = seed;
}

void DelayMs(uint16_t del)
{
    for(uint16_t i=0; i<del; i++)
       __delay_ms(1);
}