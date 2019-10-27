/*
 * File:   chipRTC.c
 * Author: Quentin
 *
 * Created on 09 June 2019, 13:27
 */


#include <xc.h>
#include <stdbool.h>
#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP
#define _XTAL_FREQ 8000000


void delay_milliseconds(unsigned int);
void delay_microseconds(unsigned int);
void LCDCommand(unsigned char);
void LCDData(unsigned char);
void LCDdisplay(unsigned char, unsigned char[]);

void startI2C(void);
void repeatstartI2C(void);
void writeinit(void);
void readinit(void);
void writebyte(unsigned char);
unsigned char readbyte(void);
void stopI2C(void);

void turnonLCD(void);

void requesttime(void);
void displaytime(void);
void savetime(unsigned char byte, unsigned char adr);

void __interrupt(low_priority) interrupthandler(void);

unsigned char time[9]={0x30 , 0x30, 0x3A, 0x30, 0x30, 0x3A, 0x30, 0x30, '\0'};

#define RS PORTEbits.RE0
#define RW PORTEbits.RE1
#define EN PORTEbits.RE2

#define MODE PORTBbits.RB0
#define UP PORTBbits.RB1
#define DOWN PORTBbits.RB2

#define SCL PORTCbits.RC3
#define SDA PORTCbits.RC4

#define SLAVE_ADR_WRITE 0xA0
#define SLAVE_ADR_READ 0xA1

#define CLK_SEC_ADR 0x02
#define CLK_MIN_ADR 0x03
#define CLK_HOUR_ADR 0x04
#define CLK_DATE_YEAR_ADR 0x05
#define CLK_MONTH_ADR 0x06


void main(void) {
    
    ANSELD = 0;
    TRISD = 0;
    
    ANSELE = 0;
    TRISE = 0;
    
    ANSELB = 0;
    TRISB = 0b00010111;
    
    ANSELC = 0;
    TRISCbits.RC3 = 1;
    TRISCbits.RC4 = 1;
                  
    SSP1CON1bits.SSPEN = 1;
    SSP1CON1bits.SSPM = 0b1000;
    SSP1ADD = 0x0F;
       
    INTCONbits.GIE = 1;
    INTCONbits.RBIE = 1;
    INTCONbits.INT0IE = 1;
    INTCON3bits.INT1IE = 1;
    INTCON3bits.INT2IE = 1;
    IOCBbits.IOCB4 = 1;
    
    INTCONbits.RBIF = 0;  
    INTCONbits.INT0IF = 0;
    INTCON3bits.INT1IF = 0;
    INTCON3bits.INT2IF = 0;
    INTCON2bits.INTEDG0 = 0;
    INTCON2bits.INTEDG1 = 0;
    INTCON2bits.INTEDG2 = 0;
    
    turnonLCD();
    
    while(1);
 
    return;
}

void __interrupt(low_priority) interrupthandler(void)
{
    static int counter = 0;
    static unsigned char adr = CLK_HOUR_ADR;
    
    if(INTCONbits.RBIF)
    {
        requesttime();        
        displaytime();        
        char latchtoresetRBIF = PORTB;
        INTCONbits.RBIF = 0;
    }
    
    if(INTCONbits.INT0IF)
    {               
        if(counter==0)
        {
           INTCONbits.RBIE = 0;
           IOCBbits.IOCB4 = 0;
           LCDCommand(0x0E);
           delay_milliseconds(15);
           counter+= 1;
           LCDCommand(0x80 | counter);
           delay_milliseconds(15);
        }        
        else
        {
           
           unsigned char adjust =  counter== 1 ?  0x40: 0x00;
           unsigned char bytetosave = (adjust | time[counter-1]<<4)  |  (time[counter] & 0x0F);                    
           adr = counter== 1 ? adr : --adr; 
           savetime(bytetosave,adr);
                  
           counter+= 3;
           if(counter >7)
           {
               LCDCommand(0x0C);
               delay_milliseconds(15);
               counter = 0;
               adr = CLK_HOUR_ADR;
               INTCONbits.RBIE = 1;
               IOCBbits.IOCB4 = 1;
           }
           else
           {
              LCDCommand(0x80 | (unsigned char)counter);
              delay_milliseconds(15);
           }                                
        }             
        INTCONbits.INT0IF = 0;
    }
  
    //up
    if(INTCON3bits.INT1IF)
    {
        if(counter>0 && counter <=7)
        {
           int unit = counter;
           int decimal = counter-1;
           unsigned char digits =  (time[unit] & 0x0F) + (time[decimal] & 0x0F) * 10;          
           digits++;
           digits = (unit==0x01) ? digits % 24: digits%60;    
           time[unit] = 0x30 | digits%10;
           time[decimal] = 0x30 | digits/10;
           unsigned char updateddigits[3]={time[decimal],time[unit],'\0'};
              
           LCDdisplay(0x80 | (unsigned char)decimal, updateddigits);          
           LCDCommand(0x80 | (unsigned char)decimal + 1);
        }
        INTCON3bits.INT1IF = 0;       
    }
    
    //down
    if(INTCON3bits.INT2IF)
    {
        if(counter>0 && counter <=7)
        {
           int unit = counter;
           int decimal = counter-1;
           unsigned char digits =  (time[unit] & 0x0F) + (time[decimal] & 0x0F) * 10;
           if(digits==0x00) {digits = (unit==0x01)? 0x17 : 0x3B;}
           else{digits--;}
           time[unit] = 0x30 | digits%10;
           time[decimal] = 0x30 | digits/10;
           unsigned char updateddigits[]={time[decimal],time[unit],'\0'};
           
           LCDdisplay(0x80 | (unsigned char)decimal, updateddigits);          
           LCDCommand(0x80 | (unsigned char)decimal + 1);
        } 
        INTCON3bits.INT2IF = 0;
    }
      
}


void savetime(unsigned char byte, unsigned char adr)
{
    startI2C();
    writeinit();
    writebyte(adr);
    writebyte(byte);
    stopI2C();
}

void requesttime(void)
{  
    startI2C();
    writeinit();
    writebyte(0x02);
    repeatstartI2C();
    readinit();
    for(int i = 7; i>0; i-=3)
    {
        unsigned char temp;
        temp=readbyte();
        temp = (i==1)? temp & 0x3F  : temp; 
        time[i] = 0x30 | (temp & 0x0F);
        time[i-1] = 0x30 | temp>>4;
    }

    stopI2C();
}

void displaytime(void)
{
     LCDdisplay(0x80, &time[0]);
}

void LCDdisplay(unsigned char cursorpos, unsigned char* temp)
{
    LCDCommand(cursorpos);
    delay_milliseconds(15);
    do
    {
        LCDData(*temp++);
        delay_milliseconds(15);
    }while(*temp != '\0'); 
    

    
}

void turnonLCD(void)
{
    EN = 0;
    delay_milliseconds(250);
    LCDCommand(0x38);
    delay_milliseconds(250);
    LCDCommand(0x01);
    delay_milliseconds(15);
    LCDCommand(0x0C);
    delay_milliseconds(15);
}

void writebyte(unsigned char byte)
{
    do
    {
        PIR1bits.SSP1IF = 0;
        SSP1BUF = byte;  
        while(!PIR1bits.SSP1IF);
    }while(SSP1CON2bits.ACKSTAT); 
}

unsigned char readbyte(void)
{
    unsigned char recbyte;
    
    PIR1bits.SSP1IF = 0;
    SSP1CON2bits.RCEN = 1;
    while(!PIR1bits.SSP1IF);
    recbyte = SSP1BUF;
    
    PIR1bits.SSP1IF = 0;
    SSP1CON2bits.ACKDT = 0; 
    SSP1CON2bits.ACKEN = 1;
    while(!PIR1bits.SSP1IF);
    
    return recbyte;
}

void startI2C(void)
{
    do
    {
        PIR1bits.SSP1IF = 0;
        SSP1STATbits.S = 0;
        SSP1CON2bits.SEN = 1;       
        while(!PIR1bits.SSP1IF);
    }while(!SSP1STATbits.S);    
}

void repeatstartI2C(void)
{
    do
    {
        PIR1bits.SSP1IF = 0;
        SSP1STATbits.S = 0;
        SSP1CON2bits.RSEN = 1;       
        while(!PIR1bits.SSP1IF);
    }while(!SSP1STATbits.S);    
}

void stopI2C(void)
{
    do
    {
        PIR1bits.SSP1IF = 0;
        SSP1STATbits.P = 0;
        SSP1CON2bits.PEN = 1;       
        while(!PIR1bits.SSP1IF);
    }while(!SSP1STATbits.P);  
}

void writeinit(void)
{  
    do
    {
        PIR1bits.SSP1IF = 0;
        SSP1BUF = SLAVE_ADR_WRITE;  
        while(!PIR1bits.SSP1IF);
    }while(SSP1CON2bits.ACKSTAT);   
}

void readinit(void)
{  
    do
    {
        PIR1bits.SSP1IF = 0;
        SSP1BUF = SLAVE_ADR_READ;  
        while(!PIR1bits.SSP1IF);
    }while(SSP1CON2bits.ACKSTAT);   
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