/*
 * File:   voltmeter.c
 * Author: Quentin
 *
 * Created on 14 April 2019, 16:26
 */


#include <xc.h>
#include <stdint.h>

#pragma config MCLRE= EXTMCLR, WDTEN=OFF, FOSC=HSHP

#define _XTAL_FREQ 8000000

const uint8_t LATC_output[]={0x01, 0x02, 0x04, 0x08, 0x10};

void main(void) {
    
    ANSELAbits.ANSA0 = 1;                //digital input buffer disabled
    TRISAbits.RA0 = 1;                   //RA0 configured as an input
    TRISC = 0;                           //PORTC configured as an output
    
    //ADC converter set up
    ADCON0bits.CHS = 0b00000;            //AN0 channel selected
    ADCON0bits.ADON = 1;                 //ADC enabled
    ADCON1bits.PVCFG = 0b00;             //Vref+ connected to Vdd
    ADCON1bits.NVCFG = 0b00;             //Vref- connected to Vss
    ADCON2bits.ADFM = 1;                 //result right justified
    ADCON2bits.ACQT = 0b100;             //acquisition time = 8 TAD
    ADCON2bits.ADCS = 0b011;             //TAD = 1.7 us (FRC)) 
    
    ADCON0bits.GO = 1;                   //ADC conversion starts
    
    uint16_t result;
    
    while(1)
    {       
        while(ADCON0bits.nDONE);         //wait for acquisition voltage + conversion time
            
        result = ((ADRESL | ADRESH << 8) * 5) >> 10;
        LATC = LATC_output[result];
  
        ADCON0bits.GO = 1;               //relaunch a new conversion
    }
       
    return;
}
