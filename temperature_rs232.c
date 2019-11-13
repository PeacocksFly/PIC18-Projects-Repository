/*
 * File:   rs232.c
 * Author: Quentin
 *
 * Created on 22 May 2019, 20:21
 */


#include <xc.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

void __interrupt(low_priority) myADC(void);
uint8_t strlen(uint8_t* s);
void reverse(uint8_t* s);
void WriteToTerminal(uint8_t* p);

uint32_t temperature;
uint8_t text[16];

void main(void) {
    
    TRISBbits.RB0 = 1;                   //RB0 as an input
    ANSELBbits.ANSB0 = 1;                //digital input disabled                           
      
    TRISCbits.RC6 = 0;                   //RC6 as an output
    TRISCbits.RC7 = 1;                   //RC7 as an input
    ANSELC = 0;                          //digital enabled
    
    //RS232 configuration
    RCSTA1bits.SPEN = 1;                 //serial port enabled
    TXSTA1bits.SYNC = 0;                 //EUSART asynchronous mode selected
    BAUDCON1bits.BRG16 = 0;              //8-bit baud rate generator
    SPBRG1 = 0x0C;                       //baud rate = 9600
    TXSTA1bits.TX9 = 0;                  //9 bits transmission disabled
    TXSTA1bits.TXEN = 1;                 //transmit bit enabled
    TXSTA1bits.BRGH = 0;                 //baud rate low speed (speed not doubled)
      
    //interrupt configuration 
    INTCONbits.GIE = 1;                  //general interrupt enabled      
    INTCONbits.PEIE = 1;                 //peripheral interrupt enabled
    PIR1bits.ADIF = 0;                   //dac interrupt flag bit reset
    PIE1bits.ADIE = 1;                   //dac interrupt enabled
    
    //ADC converter set up
    ADCON0bits.CHS = 0b01100;            //AN12 channel selected
    ADCON0bits.ADON = 1;                 //ADC enabled
    ADCON1bits.PVCFG = 0b00;             //Vref+ connected to Vdd
    ADCON1bits.NVCFG = 0b00;             //Vref- connected to Vss
    ADCON2bits.ADFM = 1;                 //result right justified
    ADCON2bits.ACQT = 0b100;             //acquisition time = 8 TAD
    ADCON2bits.ADCS = 0b011;             //TAD = 1.7 us (FRC)) 
    
    ADCON0bits.GO = 1;                   //ADC conversion starts
            
    while(1);

    return;
}


void __interrupt(low_priority) myADC(void)
{
    if(PIR1bits.ADIF)
    {
        temperature = ((ADRESL | (uint32_t)ADRESH << 8) * 5000) >> 10; 

        uint8_t *p = text;
        *p++ = 0x30 |(uint8_t)(temperature%10); //one digit after comma
        temperature/= 10;
        *p++ = 0x2E;                            //comma in ascii 
        
        do{      
           *p++ = 0x30 |(uint8_t)(temperature%10);
        }while((temperature/=10) > 0);     
        *p = '\0';                              //null character 
        
        reverse(text);                          //text reversed before display
        WriteToTerminal(text);                  //write to lcd
                
        while(!PIR1bits.TX1IF);
        TXREG1 = 0x0D;                          //carriage return
       
        __delay_ms(1000);
       
        PIR1bits.ADIF = 0;                      //interrupt flag reset
        ADCON0bits.GO = 1;                      //go bit is low after a conversion so that it needs to be set again
    }

}

void WriteToTerminal(uint8_t* p)
{
     while(*p)
     {
         while(!PIR1bits.TX1IF);        //wait for TX1IF to be set to send next byte
         TXREG1 = *p++;
     }
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

