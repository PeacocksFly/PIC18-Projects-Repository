/*
 * File:   humidity.c
 * Author: Quentin
 *
 * Created on 10 May 2019, 11:19
 */


#include <xc.h>
#include <stdbool.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000

#define EN PORTEbits.RE2
#define RW PORTEbits.RE1
#define RS PORTEbits.RE0

#define SCK PORTCbits.RC0
#define DATA PORTCbits.RC1
#define DIR_CLCK TRISCbits.RC0
#define DIR_DATA TRISCbits.RC1

void LCDCommand(unsigned char);
void LCDData(unsigned char);
void Delay_MilliSeconds(int);
void Delay_MicroSeconds(int);
void TransmissionStart();
void ResetSequence();
bool AskMeasure(char);
int GetMeasure(bool, unsigned char );
void ACKAcknowledge();
bool CRCPass(char, unsigned int, unsigned char);
void ClockingResponse(char*);

const unsigned char lookUpTable[256] = {
     0, 49, 98, 83, 196, 245, 166, 151, 185, 136, 219, 234, 125, 76, 31, 46,
     67, 114, 33, 16, 135, 182, 229, 212, 250, 203, 152, 169, 62, 15, 92, 109,
     134, 183, 228, 213, 66, 115, 32, 17, 63, 14, 93, 108, 251, 202, 153, 168,
     197, 244, 167, 150, 1, 48, 99, 82, 124, 77, 30, 47, 184, 137, 218, 235,
     61, 12, 95, 110, 249, 200, 155, 170, 132, 181, 230, 215, 64, 113, 34, 19,
     126, 79, 28, 45, 186, 139, 216, 233, 199, 246, 165, 148, 3, 50, 97, 80,
     187, 138, 217, 232, 127, 78, 29, 44, 2, 51, 96, 81, 198, 247, 164, 149,
     248, 201, 154, 171, 60, 13, 94, 111, 65, 112, 35, 18, 133, 180, 231, 214,
     122, 75, 24, 41, 190, 143, 220, 237, 195, 242, 161, 144, 7, 54, 101, 84,
     57, 8, 91, 106, 253, 204, 159, 174, 128, 177, 226, 211, 68, 117, 38, 23,
     252, 205, 158, 175, 56, 9, 90, 107, 69, 116, 39, 22, 129, 176, 227, 210,
     191, 142, 221, 236, 123, 74, 25, 40, 6, 55, 100, 85, 194, 243, 160, 145,
     71, 118, 37, 20, 131, 178, 225, 208, 254, 207, 156, 173, 58, 11, 88, 105,
     4, 53, 102, 87, 192, 241, 162, 147, 189, 140, 223, 238, 121, 72, 27, 42,
     193, 240, 163, 146, 5, 52, 103, 86, 120, 73, 26, 43, 188, 141, 222, 239,
     130, 179, 224, 209, 70, 119, 36, 21, 59, 10, 89, 104, 255, 206, 157, 172,
};

float output __at(0x050);

void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
    
    ANSELE = 0;  
    
    TRISEbits.RE0 = 0;
    TRISEbits.RE1 = 0;
    TRISEbits.RE2 = 0;
    
    ANSELC = 0;
    DIR_CLCK = 0;
    SCK = 0;
    
    unsigned char command;
    unsigned char commandTemp = 0x03;
    unsigned char commandHum = 0x05;
    
 //   float humidity;
    float temperature;
    
    LCDCommand(0x0C);
    Delay_MilliSeconds(250); 
    EN = 0;
    
    while(1)
    {
        ResetSequence();
        TransmissionStart();
        command = commandTemp;
        //command = commandHum;
        if(AskMeasure(command))
        {   
            while(DATA==1)
            {
            }
            Delay_MicroSeconds(100);
            unsigned int result = GetMeasure(true, command);
            
            if(result == 0xF000)
            {
               LCDCommand(0x01);
               Delay_MilliSeconds(250);
               LCDCommand(0x80);
               Delay_MilliSeconds(250);

               LCDData(0x43);
               Delay_MilliSeconds(15);
               LCDData(0x52);
               Delay_MilliSeconds(15);
               LCDData(0x43); 
               Delay_MilliSeconds(1000);
            }
            else              
            {
                if(command == commandTemp)
                {
                    output = -40.1f + (result * 0.01f );
                }
                else
                {
                    output =  - (0.0000015955f * result * result) + (0.0367f * result) -2.0468f ;  
                }
                
                LCDCommand(001);
                Delay_MilliSeconds(250);
                LCDCommand(0x80);
                Delay_MilliSeconds(250);

                unsigned int temp;
                unsigned int modulo;

                temp =  ((int) output)/1000;
                modulo = ((int) output)%1000;
                LCDData(temp | 0x30);
                Delay_MilliSeconds(15);

                temp =  modulo/100;
                modulo = modulo%100;
                LCDData(temp | 0x30);
                Delay_MilliSeconds(15);

                temp =  modulo/10;
                modulo = modulo%10;
                LCDData(temp | 0x30);
                Delay_MilliSeconds(15);

                LCDData(modulo | 0x30);            
                Delay_MilliSeconds(1000);
            
            }
            
        }
        else
        {
           LCDCommand(0x01);
           Delay_MilliSeconds(250);
           LCDCommand(0x80);
           Delay_MilliSeconds(250);

           LCDData(0x45);
           Delay_MilliSeconds(15);
           LCDData(0x52);
           Delay_MilliSeconds(15);
           LCDData(0x52); 
           Delay_MilliSeconds(15);
        }
    }    
    return;
}

void ResetSequence()
{
    DIR_DATA = 1;
    for(char i = 0; i<10; i++)
    {
        SCK = 1;
        Delay_MicroSeconds(1);       
        SCK = 0;
        Delay_MicroSeconds(1);
    }
}

void TransmissionStart()
{
    DIR_DATA = 1;
    Delay_MicroSeconds(1);
    SCK = 0;
    Delay_MicroSeconds(1);
    SCK = 1;
    Delay_MicroSeconds(1);
    DIR_DATA = 0;
    DATA = 0;
    Delay_MicroSeconds(1);
    SCK = 0;
    Delay_MicroSeconds(1);
    SCK = 1;
    Delay_MicroSeconds(1);
    DIR_DATA = 1;
    Delay_MicroSeconds(1);   
    SCK = 0;
    Delay_MicroSeconds(1);
    DIR_DATA = 0;
    DATA = 0;
    
    LCDCommand(0x01);
    Delay_MilliSeconds(250);
    LCDCommand(0x80);
    Delay_MilliSeconds(250);
    
    LCDData(0x54);
    Delay_MilliSeconds(15);
    LCDData(0x53);
    Delay_MilliSeconds(15);
    LCDData(0x20);
    Delay_MilliSeconds(15);
    LCDData(0x4F);
    Delay_MilliSeconds(15);
    LCDData(0x4B);
    Delay_MilliSeconds(1000);
}

bool AskMeasure(char command)
{
    for(char i = 0; i<8; i++)
    {
       char newBit = 0x01 & (command >> (7-i));
       if(newBit == 0)
       {
           DIR_DATA = 0;
           DATA = 0;
       }
       else
       {
           DIR_DATA = 1;
       }
       Delay_MicroSeconds(1);
       SCK = 1;
       Delay_MicroSeconds(1);
       SCK = 0;
       Delay_MicroSeconds(1);
    }
    
    DIR_DATA = 1;
    Delay_MicroSeconds(5);
    
    if(DATA == 0)
    {
        SCK = 1;
        Delay_MicroSeconds(1);
        SCK = 0;
        Delay_MicroSeconds(1);
        return true;
    }   
    else
    {
       return false;    
    }      
}

int GetMeasure(bool checkCRC, unsigned char command)
{
     
    char byte = 0x00;
    unsigned int data = 0x0000;

    ClockingResponse(&byte);
    data = byte;
    data = data << 8;
    byte = 0x00;
    ACKAcknowledge();
    
    ClockingResponse(&byte);
    data = data | byte;    
    if(checkCRC == true)
    {
        ACKAcknowledge();
        byte = 0x00;
        ClockingResponse(&byte);
        
        if(CRCPass(byte, data, command)== false)
        {
            return 0xF000;
        }
    }
   
    return data;
       
}

void ACKAcknowledge()
{
    DIR_DATA = 0;
    DATA=0;
    Delay_MicroSeconds(1);
    SCK = 1;
    Delay_MicroSeconds(1);
    SCK = 0;
    Delay_MicroSeconds(1);
    DIR_DATA = 1;
}

void ClockingResponse(char *byte)
{
    for(char i = 0; i<8; i++)
    {
            SCK = 1;
            Delay_MicroSeconds(1);
            if(DATA==1)
            {
                *byte = *byte | (0x80 >> i);
            }              
            SCK = 0;
            Delay_MicroSeconds(1);
    }
}

bool CRCPass(char CRC, unsigned int data, unsigned char command)
{
    unsigned char reg = 0x00;  //default
    unsigned char temp;
      
    reg = lookUpTable[reg ^ command];
    temp = (data >> 8);
    reg = lookUpTable[reg ^ temp];
    temp = data;
    reg = lookUpTable[reg ^ temp];
    
    return (CRC == reg)? true : false;
    
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