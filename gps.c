/*
 * File:   gps.c
 * Author: Quentin
 *
 * Created on 26 May 2019, 10:26
 */


#include <xc.h>
#include <stdbool.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

void Delay_MilliSeconds(int ms);
void Delay_MicroSeconds(int us);
void LCDCommand(unsigned char cmd);
void LCDData(unsigned char cmd);
void WriteToLCD(const char tab[]);
void Parse(char*,  char*,  char*);
void __interrupt(low_priority) UARTInterrupt(void);

#define RS PORTEbits.RE0
#define RW PORTEbits.RE1
#define EN PORTEbits.RE2

char GPRMCInfo[82];
char *p;
bool searchDelimiter = true;
bool fillingTab = false;
bool fullTab = false;

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
    
    ANSELC = 0;
    TRISCbits.RC6 = 0;
    TRISCbits.RC7 = 1;
    
    ANSELE = 0;
    TRISEbits.RE0 = 0;
    TRISEbits.RE1 = 0;
    TRISEbits.RE2 = 0;
    
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIE1bits.RC1IE = 1;
    
    SPBRG1 = 0x0C;
    TXSTA1bits.TX9 = 0;
    TXSTA1bits.TXEN = 1;      
    TXSTA1bits.SYNC = 0;
    TXSTA1bits.BRGH = 0;
    
    RCSTA1bits.SPEN = 1;
    RCSTA1bits.RX9 = 0;
    RCSTA1bits.CREN = 1;
    
    LCDCommand(0x0C);
    Delay_MilliSeconds(250); 
    LCDCommand(0x38);
    Delay_MilliSeconds(250);
    EN = 0;
    
    char time[8];
    char longitude[10];
    char latitude[10];
    
    while(1)
    {       
        while(!fullTab)
        {}
        
        Parse(time, longitude, latitude);
        
        LCDCommand(0x01);
        Delay_MilliSeconds(250);
        
        LCDCommand(0x80);
        Delay_MilliSeconds(250);
        WriteToLCD("GPS Position");
        Delay_MilliSeconds(15);
        
        //GPS
               
        LCDCommand(0xC0);
        Delay_MilliSeconds(250);
        WriteToLCD("Time: ");
        Delay_MilliSeconds(15);
        WriteToLCD(time);
        Delay_MilliSeconds(15);
        //Time
        
        LCDCommand(0x90);
        Delay_MilliSeconds(250);
        WriteToLCD("Long: ");
        Delay_MilliSeconds(15);
        WriteToLCD(longitude);
        Delay_MilliSeconds(15);
        //Longitude
    
        LCDCommand(0xD0);
        Delay_MilliSeconds(250); 
        WriteToLCD("Lat: ");
        Delay_MilliSeconds(15);
        WriteToLCD(latitude);
        Delay_MilliSeconds(100);
        //Latitude

        searchDelimiter = true;
        fullTab = false;     
        fillingTab = false;
    }
    
            
    return;
}

void Parse(char* time, char* longitude, char* latitude)
{
   
    //Time
    char index = 0;
    char cnt = 0;
    while(GPRMCInfo[index++] != 0x2C);
    
    while(GPRMCInfo[index] !=0x2C)
    {
        if(GPRMCInfo[index] ==0x2E)
        {
            while(GPRMCInfo[index++] != 0x2C);
            break;
        }
        *time++ = GPRMCInfo[index++];
        cnt++;
        if(cnt==2)
        {
            *time++ = 0x3A;
            cnt = 0;
        }
    }
    *--time = ' ';
    
    //Longitude
    while(GPRMCInfo[index++] != 0x2C);
    
    cnt = 0;
    while(GPRMCInfo[index] != 0x2C)
    {
        if(cnt<6)
        {
            if(GPRMCInfo[index] != 0x2E)
            {
                *longitude++ = GPRMCInfo[index];
                 cnt++;
                 if(cnt==2)
                 {
                     *longitude++ = 0x64;
                 }
                 if(cnt==4)
                 {
                     *longitude++ = 0x2E;
                 }
             }       
        }
        index++;
    }
    index++;
    
    while(GPRMCInfo[index] != 0x2C)
    {
        *longitude++ = GPRMCInfo[index++];
    }
    index++;  
    
    //Latitude
    cnt = 0;
    while(GPRMCInfo[index] != 0x2C)
    {
        if(cnt<6)
        {
            if(GPRMCInfo[index] != 0x2E)
            {
                *latitude++ = GPRMCInfo[index];
                 cnt++;
                 if(cnt==3)
                 {
                     *latitude++ = 0x64;
                 }
                 if(cnt==5)
                 {
                     *latitude++ = 0x2E;
                 }
             }           
        } 
        index++;
    }
    index++;   
    while(GPRMCInfo[index] != 0x2C)
    {
        *latitude = GPRMCInfo[index++];
    }
   
}


void __interrupt(low_priority) UARTInterrupt(void)
{
      if(PIR1bits.RC1IF == 1)
      {
         char byte = RCREG1; 
         
         if(searchDelimiter)
         {
             if(byte == 0x24)
             {
                 p = &GPRMCInfo[0];
                 *p = byte;
                 searchDelimiter = false;
                 p++;
             }            
         }
         else
         {
             if(!fullTab)
             {
                 *p = byte;
                 if(!fillingTab)
                 {
                     if(p == &GPRMCInfo[5])
                     {
                             if(GPRMCInfo[0]== 0x24 &&
                                GPRMCInfo[1]== 0x47 &&
                                GPRMCInfo[2]== 0x50 &&
                                GPRMCInfo[3]== 0x52 &&
                                GPRMCInfo[4]== 0x4D &&
                                GPRMCInfo[5]== 0x43)
                             {
                                  fillingTab = true;
                                  p++;
                             }
                             else
                             {
                                  searchDelimiter = true;
                             }
                     }
                     else
                     {
                         p++;
                     }
                 }
                 else
                 {   
                     if(byte == 0x0D)
                     {
                         fullTab = true;
                     }
                     p++;
                 }                 
             } 
         }
      }
}


void WriteToLCD(const char tab[])
{
    while(*tab!='\0')
    {
        LCDData(*tab);
        Delay_MilliSeconds(15);
        tab++;
    }

}


void LCDCommand(unsigned char cmd)
{
    PORTD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    Delay_MilliSeconds(1);
    EN = 0;
  
}

void LCDData(unsigned char cmd)
{
    PORTD = cmd;
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

void Delay_MicroSeconds(int us)
{
    for(int i = 0; i< us; i++)
    {
        __delay_us(1);
    }   
}