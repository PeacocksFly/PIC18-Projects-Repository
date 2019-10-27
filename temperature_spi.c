/*
 * File:   tempSPI.c
 * Author: Quentin
 *
 * Created on 06 June 2019, 12:23
 */


#include <xc.h>
#include <pic18f45k22.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000


void delay_milliseconds(unsigned int);
void delay_microseconds(unsigned int);
void LCDCommand(unsigned char);
void LCDData(unsigned char);
void writetosensor(const unsigned char, const unsigned char);
void readfromsensor(const unsigned char, char*);
float tempconversion(const char*);
void ASCIIconversion(float, char*, const int);
void LCDdisplay(char*);
int power(const int, int);
int strlen(char []);
void reverse(char []);

#define RS PORTEbits.RE0
#define RW PORTEbits.RE1
#define EN PORTEbits.RE2

#define CE PORTCbits.RC2
#define CK PORTCbits.RC3
#define IN PORTCbits.RC4
#define OUT PORTCbits.RC5

#define ADR_CTRL_WRITE 0x80
#define ADR_CTRL_READ 0x00
#define ADR_TEMP_MSB 0x02
#define ADR_TEMP_LSB 0x01


void main(void) {
    
    ANSELE = 0;
    TRISE = 0;
    
    ANSELD = 0;
    TRISD = 0;
    
    ANSELC = 0;
    TRISCbits.RC2 = 0;
    TRISCbits.RC3 = 0;
    TRISCbits.RC4 = 1;
    TRISCbits.RC5 = 0;
    
    SSP1CON1bits.SSPEN = 1;
    SSP1CON1bits.SSPM3=0;
    SSP1CON1bits.SSPM2=0;
    SSP1CON1bits.SSPM1=0;
    SSP1CON1bits.SSPM0=0;
    SSP1STATbits.SMP = 0;
    SSP1STATbits.CKE = 0;
    
    LCDCommand(0x0C);
    delay_milliseconds(250); 
    EN = 0;
      
    CE = 0;
    char readings[3];//={0x19, 0x00, 0x00};
    float temperature;
    char ASCIItab[8];
    
    while(1)
    {
       writetosensor(ADR_CTRL_WRITE, 0x04);       
       delay_milliseconds(150);
       readfromsensor(ADR_TEMP_MSB, &readings[0]);
       
       LCDCommand(0x01);
       delay_milliseconds(250);
       LCDCommand(0x80);
       delay_milliseconds(250);
       
       temperature = tempconversion(&readings[0]);
       ASCIIconversion(temperature, &ASCIItab[0], 2);
   
       LCDdisplay(&ASCIItab[0]);
       
       delay_milliseconds(500);
    }
            
    return;
}


void ASCIIconversion(float tmp, char* ASCIItab, const int resolution)
{
    char* starttab = ASCIItab;

    char sign = 0;
    if(tmp < 0)
    {       
       tmp = -tmp;
       sign = 1;
    }
    
    int integer = (int)(tmp * (float)power(10, resolution));
         
    for(int p=resolution; p>0;--p)
    {
       *ASCIItab++ = (0x30 | integer%10);  
       integer=integer/10;
    }
    
    *ASCIItab++ = 0x2E;

    do
    {
        *ASCIItab++=(0x30 |integer%10);    
    }while((integer=integer/10) > 0);
        
    if(sign==1)
    {
       *ASCIItab++ = 0x2D;
    }
    
    *ASCIItab = '\0';
    
    reverse(starttab);
    
}



int strlen(char s[])
{
    int i;
    i = 0;
    
    while (s[i] != '\0')
    {
        i++;
    }
      
    return i;
}



void reverse(char s[])
{
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) 
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }   
}


void LCDdisplay(char* temp)
{
    do
    {
        LCDData(*temp);
        delay_milliseconds(15);
    }while(*temp++ != '\0');    
}

int power(const int bas, int n)
{
    int p;
    for(p = 1; n>0; --n)
    {
        p = p * bas;
    }        
    return p;
}


float tempconversion(const char* readings)
{
    float temp;
    
    if((*readings & 0x80) == 0x80)
    {
       temp = -(1.0 + (char)~*readings++) ;
    }
    else
    {
       temp = *readings++;
    }

    temp = temp + (*readings >> 6) * 0.25;

    return temp;
}


void writetosensor(const unsigned char adr, const unsigned char ctrlreg)
{
    CE = 1;
       
    SSP1BUF = adr;
    while(SSP1STATbits.BF == 0);
    SSP1BUF = ctrlreg;    
    while(SSP1STATbits.BF == 0);       
    
    CE = 0;
}


void readfromsensor(const unsigned char adr, char* readings)
{
    CE = 1;
       
    SSP1BUF = adr;
    while(SSP1STATbits.BF == 0);
    
    for(unsigned char i=0; i<3; i++)
    {
        SSP1BUF = 0xAA;     //dummy
        while(SSP1STATbits.BF == 0);
        *readings++ = SSP1BUF;
    }
    
    CE = 0;     
}


void LCDCommand(unsigned char cmd)
{
    PORTD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    delay_milliseconds(1);
    EN = 0;
}

void LCDData(unsigned char cmd)
{
    PORTD = cmd;
    RS = 1;
    RW = 0;
    EN = 1;
    delay_milliseconds(1);
    EN = 0;
}

void delay_milliseconds(unsigned int ms)
{
    for(unsigned int i=0; i<ms; i++)
    {
        __delay_ms(1);
    }
}

void delay_microseconds(unsigned int us)
{
    for(unsigned int i=0; i<us; i++)
    {
        __delay_us(1);
    }
}
