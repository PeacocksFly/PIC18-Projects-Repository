/*
 * File:   chipRTC.c
 * Author: Quentin
 *
 * Created on 09 June 2019, 13:27
 */


#include <xc.h>
#include <stdint.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

void LCDCommand(uint8_t cmd);
void LCDData(uint8_t cmd);
void writeToLCD(uint8_t* p, uint8_t pos);
void startI2C(void);
void repeatStartI2C(void);
void writeInit(void);
void readInit(void);
void writeByte(uint8_t);
uint8_t readByte(void);
void stopI2C(void);
void requestTime(void);
void displayTime(void);
void saveTime(uint8_t, uint8_t );

void __interrupt(low_priority) interruptHandler(void);

uint8_t time[9]={0x30, 0x30, 0x3A, 0x30, 0x30, 0x3A, 0x30, 0x30, '\0'};

#define _XTAL_FREQ 8000000

#define RS LATEbits.LATE0
#define RW LATEbits.LATE1
#define EN LATEbits.LATE2

#define SLAVE_ADR_WRITE 0xA0
#define SLAVE_ADR_READ 0xA1

#define CLK_SEC_ADR 0x02                 //register addresses for the pcf 8583 
#define CLK_MIN_ADR 0x03
#define CLK_HOUR_ADR 0x04
#define CLK_DATE_YEAR_ADR 0x05
#define CLK_MONTH_ADR 0x06

void main(void) {
    
    TRISD = 0;
    TRISE = 0;
    
    ANSELB = 0;
    TRISB = 0b00010111;
    
    ANSELC = 0;
    TRISCbits.RC3 = 1;
    TRISCbits.RC4 = 1;               
    
    SSP1CON1bits.SSPM = 0b1000;             // I2C Master mode, clock = FOSC / (4 * (SSPxADD+1))
    SSP1ADD = 0x03;                         // clock = FOSC / 16
    SSP1CON1bits.SSPEN = 1;                 //enables i2c pins
       
    INTCONbits.GIE = 1;                     //general interrupt enabled
    INTCONbits.RBIE = 1;                    //interrupt-on-change enabled
    IOCBbits.IOCB4 = 1;                     //IOCB pin needs to be enabled for the interrupt-on-change to work
    INTCONbits.INT0IE = 1;                  //interrupt on RB0 enabled
    INTCON3bits.INT1IE = 1;                 //interrupt on RB1 enabled
    INTCON3bits.INT2IE = 1;                 //interrupt on RB2 enabled
    PIR1bits.SSP1IF = 0;                    //reset MSSP interrupt flag bit
    
    INTCONbits.RBIF = 0;                    //RBIF reset
    INTCONbits.INT0IF = 0;                  //interrupt flag on RB0 reset
    INTCON3bits.INT1IF = 0;                 //interrupt flag on RB1 reset
    INTCON3bits.INT2IF = 0;                 //interrupt flag on RB2 reset
    INTCON2bits.INTEDG0 = 0;                //interrupt on falling edge
    INTCON2bits.INTEDG1 = 0;
    INTCON2bits.INTEDG2 = 0;
    
    EN = 0;  
    __delay_ms(100);
    LCDCommand(0x38);                   //LCD 2 lines, 5x7 matrix
    __delay_ms(100);
    LCDCommand(0x0C);                   //display on, cursor off
    __delay_ms(5);
       
    writeToLCD((uint8_t*)"Real Time Clock", 0x80);
    
    while(1);
 
    return;
}

void __interrupt(low_priority) interrupthandler(void)
{
    static uint8_t counter = 0;
    static uint8_t adr = CLK_HOUR_ADR;             
    static uint8_t resetRBIF;
    
    if(INTCONbits.RBIF)                            //interrupt-on-change - a 1Hz pulse applied on pin RB4
    {                                              //the time is requested and then displayed on the LCD
        requestTime();        
        displayTime();        
        resetRBIF = PORTB;                         //PORT B needs to be read in order to reset RBIF
        INTCONbits.RBIF = 0;                       //reset RBIF
    }
    
    //update time
    if(INTCONbits.INT0IF)                          //change the time
    {               
        if(counter==0)
        {
           INTCONbits.RBIE = 0;                    //disable interrupt-on-change
           IOCBbits.IOCB4 = 0;
           
           LCDCommand(0x0E);                       //display on cursor blinking
           __delay_us(50);
           counter+= 1;
           LCDCommand(0xC0 | counter);             //cursor under hour second digit
           __delay_us(50);
        }        
        else
        {          
                                                   //if counter = 1 that means we are to save the hour digits
                                                   //and the first two MSB bits are configured 01 to have 24 hours format (see datasheet) 
           uint8_t adjust =  counter== 1 ?  0x40: 0x00;
           
           uint8_t bytetosave = (adjust | time[counter-1]<<4)  |  (time[counter] & 0x0F);                    
           adr = counter== 1 ? adr : --adr;        //address of the hour register to be decremented for minute and second
           
           saveTime(bytetosave,adr);               //the input time is sent to the RTC
                  
           counter+= 3;                            //increment the counter to update either min or sec
           if(counter >7)                          //if seconds have changed the clock has been updated 
           {
               LCDCommand(0x0C);                   //cursor removed
               __delay_us(50);
               counter = 0;                        //counter reset
               adr = CLK_HOUR_ADR;                 //address reset
               INTCONbits.RBIE = 1;                //re-enable the interrupt-on-change  
               IOCBbits.IOCB4 = 1;
           }
           else
           {
              LCDCommand(0xC0 | counter);          //move the cursor
              __delay_us(50);
           }                                
        }             
        INTCONbits.INT0IF = 0;
    }
  
    //up
    if(INTCON3bits.INT1IF)                         //increment time
    {
        if(counter>0 && counter <=7)               //if time is not in reset mode then the interrupt is skipped
        {
           uint8_t unit = counter;
           uint8_t decimal = counter-1;
                                                   //take the decimal and unit digits and convert from ascii 
                                                   //to number and then combine digits to form a number
           uint8_t digits =  (time[unit] & 0x0F) + (time[decimal] & 0x0F) * 10;          
           digits++;
                                                   //after incrementing take modulo on hours and minutes/seconds
           digits = (unit==0x01) ? digits % 24: digits%60;    
                                                   //convert in ascii
           time[unit] = 0x30 | digits%10;
           time[decimal] = 0x30 | digits/10;
           uint8_t updatedDigits[3]={time[decimal],time[unit],'\0'};
                                                   //send the updated figures to LCD and then move back the cursor
           writeToLCD(updatedDigits, 0xC0 | decimal);          
           LCDCommand(0xC0 | decimal + 1);
        }
        INTCON3bits.INT1IF = 0;       
    }
    
    //down             
    if(INTCON3bits.INT2IF)                         //decrement time
    {
        if(counter>0 && counter <=7)
        {
           uint8_t unit = counter;
           uint8_t decimal = counter-1;
                                                   //take the decimal and unit digits and convert from ascii 
                                                   //to number and then combine digits to form a number
           uint8_t digits =  (time[unit] & 0x0F) + (time[decimal] & 0x0F) * 10;
                                                   //if digits are equal to 0 then hours = 23 or min/sec = 59
           if(digits==0x00) {digits = (unit==0x01)? 0x17 : 0x3B;}
           else{digits--;}
           time[unit] = 0x30 | digits%10;
           time[decimal] = 0x30 | digits/10;
           uint8_t updatedDigits[]={time[decimal],time[unit],'\0'};
           
           writeToLCD(updatedDigits, 0xC0 | decimal);          
           LCDCommand(0xC0 | decimal + 1);
        } 
        INTCON3bits.INT2IF = 0;
    }
      
}


void saveTime(uint8_t byte, uint8_t adr)
{
    startI2C();                                       //start condition
    writeInit();                                      //write slave to transmit address
    writeByte(adr);                                   //write address of register to be written to
    writeByte(byte);                                  //write the byte into the buffer
    stopI2C();                                        //stop condition
}

void requestTime(void)
{  
    startI2C();                                        //start condition                                 
    writeInit();                                       //write slave to transmit address
    writeByte(0x02);                                   //write address of register to be read, i.e. seconds address
    repeatStartI2C();                                  //repeat start condition 
    readInit();                                        //tell the slave we wanna read from it
    for(int i = 7; i>0; i-=3)                          //save time in time array hours/min/sec
    {                                                  
        uint8_t temp;
        temp=readByte();
        temp = (i==1)? temp & 0x3F  : temp;            //if hour register clear the AM/PM format
        time[i] = 0x30 | (temp & 0x0F);                //in time array storage of different digits
        time[i-1] = 0x30 | temp>>4;
    }

    stopI2C();                                         //initiate a stop condition
}

void displayTime(void)
{
     writeToLCD(&time[0], 0xC0);
}

uint8_t readByte(void)
{
    uint8_t recbyte; 
    
    SSP1CON2bits.RCEN = 1;             //set receive enable bit to indicate a reception operation
    while(!PIR1bits.SSP1IF);           //wait for MSSP interrupt flag bit to indicate one byte received
    PIR1bits.SSP1IF = 0;               //reset MSSP interrupt flag bit
    recbyte = SSP1BUF;  
      
    SSP1CON2bits.ACKDT = 0;            //ACK to be sent to slave
    SSP1CON2bits.ACKEN = 1;            //ACK initiation
    while(!PIR1bits.SSP1IF);
    PIR1bits.SSP1IF = 0;
    
    return recbyte;
}

void readInit(void)
{   
    SSP1BUF = SLAVE_ADR_READ ;          //slave address to receive from
    while(!PIR1bits.SSP1IF);            //MSSP interrupt flag bit set by hardware after 9th clock
    PIR1bits.SSP1IF = 0;                //reset MSSP interrupt flag bit
    if(SSP1CON2bits.ACKSTAT)
        writeToLCD((uint8_t*)"Acknowledgement failed", 0xC0);
}

void stopI2C(void)
{
    
    SSP1STATbits.P = 0;                 //reset stop bit
    SSP1CON2bits.PEN = 1;               //initiate a stop condition
    while(!PIR1bits.SSP1IF);            //MSSP interrupt flag bit set by hardware following a stop condition completion 
    PIR1bits.SSP1IF = 0;                //clear MSSP interrupt flag bit     
    if(!SSP1STATbits.P)
        writeToLCD((uint8_t*)"Stop condition error", 0xC0);  
}

void startI2C(void)
{
    
    SSP1STATbits.S = 0;                 //reset start bit
    SSP1CON2bits.SEN = 1;               //initiate a start condition
    while(!PIR1bits.SSP1IF);            //MSSP interrupt flag bit set by hardware following a start condition completion 
    PIR1bits.SSP1IF = 0;                //clear MSSP interrupt flag bit
    if(!SSP1STATbits.S)
        writeToLCD((uint8_t*)"Start condition error", 0xC0);
}

void repeatStartI2C(void)
{ 
    
    SSP1STATbits.S = 0;                 //reset start bit
    SSP1CON2bits.RSEN = 1;              //initiate a restart condition  
    while(!PIR1bits.SSP1IF);            //MSSP interrupt flag bit set by hardware following a start condition completion  
    PIR1bits.SSP1IF = 0;                //clear MSSP interrupt flag bit
    if(!SSP1STATbits.S)
        writeToLCD((uint8_t*)"Restart condition error", 0xC0);
}

void writeInit(void)
{   
    SSP1BUF = SLAVE_ADR_WRITE;          //slave address to transmit
    while(!PIR1bits.SSP1IF);            //MSSP interrupt flag bit set by hardware after 9th clock
    PIR1bits.SSP1IF = 0;                //reset MSSP interrupt flag bit
    if(SSP1CON2bits.ACKSTAT)
        writeToLCD((uint8_t*)"Acknowledgement failed", 0xC0);
}

void writeByte(uint8_t byte)
{
    
    SSP1BUF = byte;                     //slave address to transmit
    while(!PIR1bits.SSP1IF);            //MSSP interrupt flag bit set by hardware after 9th clock
    PIR1bits.SSP1IF = 0;                //reset MSSP interrupt flag bit
    if(SSP1CON2bits.ACKSTAT)
        writeToLCD((uint8_t*)"Acknowledgement failed", 0xC0);
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