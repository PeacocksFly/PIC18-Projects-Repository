/*
 * File:   humidity.c
 * Author: Quentin
 *
 * Created on 10 May 2019, 11:19
 */


#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

#define _XTAL_FREQ 8000000
#define EN LATEbits.LATE2
#define RW LATEbits.LATE1
#define RS LATEbits.LATE0

#define SCK PORTCbits.RC0
#define DATA PORTCbits.RC1
#define DIR_CLCK TRISCbits.RC0
#define DIR_DATA TRISCbits.RC1
#define CMD_TMP  0x03
#define CMD_HUM 0x05

void TransmissionStart();
void ResetSequence();
bool AskMeasure(uint8_t);
uint16_t GetMeasure(bool, uint8_t );
void ACKAcknowledge();
bool CRCPass(uint8_t, uint16_t, uint8_t);
void ClockingResponse(uint8_t*);
void WriteToLCD(uint8_t* p, uint8_t pos);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t cmd);
uint8_t strlen(uint8_t* s);
void reverse(uint8_t* s);
uint8_t* DecimalToASCII(uint8_t* ascii, uint16_t dec);

const uint8_t lookUpTable[256] = {                                                //CRC look up table
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

void main(void) {
    
    TRISD = 0;
    TRISE = 0; 
    ANSELC = 0;
    
    DIR_CLCK = 0;
    SCK = 0;
    
    float output;                          //local variable to store result after conversion
    uint16_t measure;                      //local variable to store bits sent by sensor
    uint8_t cmd;                           //type of command needed: temp or humidity
    uint8_t text[16];                      //array for string to send to LCD

    EN = 0;  
    __delay_ms(100);
    LCD_Command(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCD_Command(0x0C);                   //display on, cursor off
    __delay_ms(5);
    
    WriteToLCD((uint8_t*)"Temperature", 0x80);
    
    ResetSequence();                       //if connection is lost reset sequence is used
   
    while(1)
    {  
        TransmissionStart();                   //transmission start sequence to initiate a transmission
        //cmd = CMD_TMP;
        cmd = CMD_HUM;
        if(AskMeasure(cmd))
        {   
            while(DATA==1);                    //sensor pulls data line down when measurement is completed

            __delay_us(100);                            
            measure = GetMeasure(true, cmd);   //retrieve the measurement
            
            if(measure == 0xFFFF)
            {
               WriteToLCD((uint8_t*)"CRC Error", 0xC0);
            }
            else              
            {
                output =  cmd == CMD_TMP ? -40.1f + measure * 0.01f :
                                                      -0.0000015955f * measure * measure + 0.0367f * measure -2.0468f ;
        
                uint8_t* p = text;     
                if(cmd == CMD_TMP)
                {
                    *p++ = 0x43;                        //letter C in ascii 
                    *p++ = 0x64;                        //letter d in ascii   
                }
                else
                {
                    *p++ = 0x48;                        //letter H in ascii 
                    *p++ = 0x52;                        //letter R in ascii               
                }                                     
                *p++ = 0x20;                            //space in ascii
                
                output *= 10;                           //to get one digit after comma
                *p++ = 0x30 |((uint16_t)output%10);      //one digit after comma
                output/= 10;
                *p++ = 0x2E;                            //comma in ascii
                 p = DecimalToASCII(p,output);          //decimal to ascii
                *p = '\0';                              //null character        
                reverse(text);                          //text reversed before display
                WriteToLCD(text, 0xC0);                 //write to lcd
                __delay_ms(100);
            }
            
        }
        else
        {
           WriteToLCD((uint8_t*)"Command Error", 0xC0);  //command wrongly sent  
        }
    }    
    
    return;
}

void ResetSequence()
{
    DIR_DATA = 1;                            //while data line is high (line as an input and pull-up resistor)
    for(char i = 0; i<10; i++)               //sck toggled  times
    {
        SCK = 1;
        __delay_us(1);  
        SCK = 0;
        __delay_us(1);
    }
}

void TransmissionStart()
{
    DIR_DATA = 1;                             //data line as input to let it pull up by the resistor
    __delay_us(1);                            //small delay (based on datasheet 0.5 us would be enough)
                                              //but would require to implement a delay with a timer
                                              //to ease the work we use the api on mplab
    SCK = 0;                                  //sck line lowered to make sure it is not high from the previous
                                              //transmission
    __delay_us(1);
    SCK = 1;                                  //sck high --> transmission start sequence really starts here
    __delay_us(1);
    DIR_DATA = 0;                             //take control of data line by making it an output
    DATA = 0;                                 //data line lowered
    __delay_us(1);
    SCK = 0;                                  //toggle sck
    __delay_us(1);
    SCK = 1;
    __delay_us(1);
    DIR_DATA = 1;                             //release data line --> end of transmission sequence
    __delay_us(1);   
    SCK = 0;                                  //sck at 0 and data at 0 to prepare line before sending cmd
    __delay_us(1);
    DIR_DATA = 0;
    DATA = 0;                                 
    
    WriteToLCD((uint8_t*)"Trans. seq. okay", 0xC0);
    __delay_ms(1000);
    WriteToLCD((uint8_t*)"                ", 0xC0);
}

bool AskMeasure(uint8_t command)
{
    for(uint8_t i = 0; i<8; i++)
    {
       uint8_t bit_to_send = 0x01 & (command >> (7-i));  //shift command bit by bit 
       if(!bit_to_send)                                  //if bit = 0, set the line as output and data = 0
       {
           DIR_DATA = 0;
           DATA = 0;
       }
       else
           DIR_DATA = 1;                 
       
       __delay_us(1);
       SCK = 1;                                          //clocking to send data line sck = 1, sck = 0
       __delay_us(1);
       SCK = 0;
       __delay_us(1);
    }
    
    DIR_DATA = 1;                                        //release the data line to receive ack
    __delay_us(5);
    
    if(DATA == 0)                                        //if proper reception sensor pulls the line down
    {
        SCK = 1;                                         //clocking to indicate we got the ack and wait 
        __delay_us(1);                                   //for the measurement
        SCK = 0;
        __delay_us(1);
        return true;                                     //return the command was correctly sent
    }   
    else
    {
       return false;                                     //issue in command 
    }      
}

uint16_t GetMeasure(bool checkCRC, uint8_t command)
{
    uint8_t byte = 0x00;
    uint16_t data = 0x0000;

    ClockingResponse(&byte);           //clock the first byte
    data = (uint16_t)byte;             //and store it in data
    data = data << 8;
    ACKAcknowledge();                  //send the ack
    
    byte = 0x00;                       //byte us is reset
    ClockingResponse(&byte);           //clock the second byte
    data = data | byte;                //and store it in data
    if(checkCRC == true)               //CRC may be used or abandoned
                                       //if data line ie maintained high CRC is ignored
    {
        ACKAcknowledge();              //by doing the ack we indicate that the crc wants to be checked
        byte = 0x00;
        ClockingResponse(&byte);       //clock the crc
        
        DIR_DATA = 1;                  //ACK acknowledge on CRC
        __delay_us(1);
        SCK = 1;
        __delay_us(1);
        SCK = 0;
        
        if(CRCPass(byte, data, command) == false)
            return 0xFFFF;             //data result impossible to obtain due to MSB idle bits in all measures
    }
   
    return data;
       
}

void ACKAcknowledge()
{
    DIR_DATA = 0;                         //to indicate sensor that one byte was received
    DATA=0;                               //we take control of the data line and we clock it
    __delay_us(1);
    SCK = 1;
    __delay_us(1);
    SCK = 0;
    __delay_us(1);
    DIR_DATA = 1;                         //finally line is released to get the new byte
}

void ClockingResponse(uint8_t *byte)
{
    for(uint8_t i=0; i<8; i++)
    {
        SCK = 1;                              //sck high plus small delay
        __delay_us(1);
        if(DATA==1)                           //if data is 1 the corresponding bit is modified
            *byte = *byte | (0x80 >> i);            
        SCK = 0;
        __delay_us(1);
    }
}

bool CRCPass(uint8_t CRC, uint16_t data, uint8_t command)   //CRC byte-wise calculation approach
{
    uint8_t reg = 0x00;                   //initialize crc register with lower nibble of status registed (0 by default)
    uint8_t temp;
      
    reg = lookUpTable[reg ^ command];     //XOR command and register and take the value in look up table
    temp = (uint8_t)(data >> 8);          //MSB byte of response
    reg = lookUpTable[reg ^ temp];        //XOR MSB and register and take the value in look up table
    temp = (uint8_t)data;                 //LSB byte of response
    reg = lookUpTable[reg ^ temp];        //XOR LSB and register and take the value in look up table
                                          //error in datasheet Reg nibbles must not be reversed
    return (CRC == reg)? true : false;   
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
    LATD = cmd;
    RS = 0;
    RW = 0;
    EN = 1;
    __delay_ms(1);
    EN = 0;
}

void LCD_Data(uint8_t cmd)
{
    LATD = cmd;
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