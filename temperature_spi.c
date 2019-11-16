/*
 * File:   tempSPI.c
 * Author: Quentin
 *
 * Created on 06 June 2019, 12:23
 */


#include <xc.h>
#include<stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

void LCDCommand(uint8_t cmd);
void LCDData(uint8_t cmd);
void writeToLCD(uint8_t* p, uint8_t pos);
void writeToSensor(const uint8_t, const uint8_t);
void readFromSensor(const uint8_t, int8_t*);
void ASCIIConversion(float, uint8_t*);
uint8_t strlen(uint8_t* s);
void reverse(uint8_t* s);

#define _XTAL_FREQ 8000000
#define RS LATEbits.LATE0
#define RW LATEbits.LATE1
#define EN LATEbits.LATE2

#define CE PORTCbits.RC2
#define CK PORTCbits.RC3
#define IN PORTCbits.RC4
#define OUT PORTCbits.RC5

#define ADR_CTRL_WRITE 0x80
#define ADR_CTRL_READ 0x00
#define ADR_TEMP_MSB 0x02
#define ADR_TEMP_LSB 0x01


void main(void) {
    
    TRISE = 0;                         //Port E and D as output
    TRISD = 0;
    
    TRISCbits.RC2 = 0;
    TRISCbits.RC3 = 0;
    TRISCbits.RC4 = 1;                 //SDI as an input while other spi pins as output
    TRISCbits.RC5 = 0;
    ANSELC = 0;                        //digital enabled
      
    //SPI configuration
    SSP1CON1bits.SSPM=0b0000;          //SPI Master mode, clock = FOSC/4
    SSP1CON1bits.SSPEN = 1;            //SPI pins enabled
    SSP1STATbits.SMP = 0;              //Input data sampled at middle of data output time  
    SSP1STATbits.CKE = 0;              //Transmit occurs on transition from Idle to active clock state
    
    EN = 0;  
    __delay_ms(100);
    LCDCommand(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCDCommand(0x0C);                   //display on, cursor off
    __delay_ms(5);
       
    writeToLCD((uint8_t*)"Temperature TC72", 0x80);
      
    CE = 0;
    int8_t readings[3];                                    //array to store MSB, LSB and control register
    float temperature;
    uint8_t text[16];
    
    while(1)
    {
       writeToSensor(ADR_CTRL_WRITE, 0x04);                 //write to the control register to clear the shutdown bit.
                                                            //continuous temperature conversion mode
       __delay_ms(150);                                     //conversion performed every 150 ms
       readFromSensor(ADR_TEMP_MSB, &readings[0]);          //3-bytes readings when the MSB address is written to the sensor
                                                            //the sensor automatically the lower addresses, i.e. LSB and control register
               
       temperature = (float)readings[0] + (readings[1] >> 6) * 0.25f;
                                                            //data is received in two bytes MSB and LSB
       ASCIIConversion(temperature, text);                  //conversion in ascii characters
       writeToLCD((uint8_t*)&text[0], 0xC0);                //display on LCD
       
       __delay_ms(1000);
       writeToLCD((uint8_t*)"           ", 0xC0);      
    }           
    return;
}

void writeToSensor(const uint8_t adr, const uint8_t ctrlreg)
{
    CE = 1;
       
    SSP1BUF = adr;                           //write address of the control register
    while(!SSP1STATbits.BF);                 //wait for full status bit to be set
    SSP1BUF = ctrlreg;    
    while(!SSP1STATbits.BF);       
    
    CE = 0;
}


void readFromSensor(const uint8_t adr, int8_t* readings)
{
    CE = 1;
        
    SSP1BUF = adr;                          //read address of MSB temperature
    while(!SSP1STATbits.BF);
    
    for(uint8_t i=0; i<3; i++)              //3-bytes data packet
    {
        SSP1BUF = 0xAA;                     //to clock in the temperature data a dummy byte put into the spi buffer
        while(!SSP1STATbits.BF);
        *readings++ = SSP1BUF; 
    }
    
    CE = 0;     
}

void ASCIIConversion(float temp, uint8_t* p)
{
    uint8_t* start = p;
    int8_t sign = 1;
    uint16_t tmp;
    
    if(temp < 0)
    {
        sign = -1;
        temp = -temp;
    }
    
    tmp = (uint16_t)(temp * 100.0f);
    
    *p++ = 0x64;                            //letter d in ascii
    *p++ = 0x20;                            //space in ascii
    
    for(uint8_t i=0; i<2;i++)
    {
        *p++ = 0x30 |(uint8_t)(tmp%10);      //two digits after comma
        tmp/= 10;   
    }
    
    *p++ = 0x2E;                            //comma in ascii 
    do{                                     //digits before comma
       *p++ = 0x30 |(uint8_t)(tmp%10);
    }while((tmp/=10) > 0);

    if(sign == -1)                      
        *p++ = 0x2D;                        //sign in ascii
    
    *p = '\0'; 
    reverse(start);
    
}

void writeToLCD(uint8_t* p, uint8_t pos)
{
     LCDCommand(pos);
     __delay_ms(5);
     while(*p)
     {
           LCDData((uint8_t)*p++);
           __delay_ms(5);
     }
}

void LCDCommand(uint8_t cmd)
{
    LATD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    __delay_us(1);
    EN = 0;
}

void LCDData(uint8_t cmd)
{
    LATD = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    __delay_us(1);
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
