// CONFIG1
#pragma config FOSC = INTOSC    
#pragma config WDTE = ON       
#pragma config PWRTE = OFF      
#pragma config MCLRE = ON    // RA3 is digital input/output (not MCLR)
#pragma config CP = OFF         
#pragma config CPD = OFF        
#pragma config BOREN = ON       
#pragma config CLKOUTEN = OFF   
#pragma config IESO = ON        
#pragma config FCMEN = ON       

// CONFIG2
#pragma config WRT = OFF        
#pragma config PLLEN = ON       
#pragma config STVREN = ON      
#pragma config BORV = LO        
#pragma config LVP = OFF        

#define _XTAL_FREQ 1000000

#include <xc.h>
#include <math.h>
//#include <stdbool.h>

// ----------- Global Variables -----------
unsigned int counter_ADC = 0;
unsigned int counter_Reset = 0;
unsigned int limit_ADC = 1;          // ~1 cycles of 1s
unsigned int limit_Reset = 40;          // ~60 cycles of 1s ~ 1 minute
unsigned char threshold3 = 590; //147 * 1023 / 255 ; 147;   // 6V Voltage must rise to this
unsigned char threshold4 = 605; //151 * 1023 / 255 ? 605     ; 151;   // 5.8V Voltage low threshold (from zener)
unsigned char adc_char = 0;
unsigned char last_adc = 0;
unsigned char voltage_rised = 0;
unsigned char currentState = 0;

// ----------- ADC Function (10-bit accuracy, returns 8-bit) -----------
unsigned int ADC_Lecture(void) {
    ADCON0bits.ADON = 1;          // Turn on ADC
    __delay_ms(2);                // Acquisition time
    ADCON0bits.GO = 1;            // Start conversion
    while (ADCON0bits.GO_nDONE); // Wait until done

    return ((ADRESH << 8) | ADRESL); // Combine high + low byte
}

// ----------- Optional WDT delay function -----------
void delayWithWDT(unsigned int cycles) {
    for (unsigned int i = 0; i < cycles; i++) {
        CLRWDT();
        __delay_ms(10);  // Keep < WDT timeout (~1s)
    }
}

// ----------- Main -----------
void main(void) {
    // --- WDT + Oscillator setup ---
    CLRWDT();
    OPTION_REGbits.PSA = 1;      
    OPTION_REGbits.PS = 0b111;   // 1:256
    INTCON = 0;
    PIE1 = 0;
    PIR1 = 0;
    OSCCON = 0b01011010;         // Internal OSC, 1MHz
    OSCCONbits.SCS1 = 1;

    // --- IO setup ---
    TRISA = 0b00000001;          // RA0 = input (ADC), others output
    LATA = 0x00;
    ANSELAbits.ANSA0 = 1;        // RA0 analog
    ADCON1bits.ADCS = 0b000;
    ADCON1bits.ADPREF = 0b00;    
    ADCON1bits.ADFM = 1;  // Right justified (10-bit)
    ADCON0bits.CHS = 0b0000;     // RA0
    APFCONbits.P1BSEL = 0;
    TRISAbits.TRISA3 = 0;    // Make RA3 an output

    // --- Initial Power ON ---
    LATAbits.LATA2 = 1;         // Start with power ON
    last_adc = ADC_Lecture();
    LATAbits.LATA2 = 0;         // Start with power ON

    delayWithWDT(1);          // Initial wait if needed

    while (1) {
        counter_ADC++;
        counter_Reset++;
            //delayWithWDT(2);          // Initial wait if needed

        LATAbits.LATA1 = 0;
        SLEEP();                // Sleep ~1s (WDT)
        NOP();

        if (counter_ADC >= limit_ADC) {
            counter_ADC = 0;
          //  LATAbits.LATA2 ^= 1;

                LATAbits.LATA1 =1;  
                adc_char = ADC_Lecture();
                LATAbits.LATA1 =0;
               if (adc_char<threshold3){ // if we reached 6V 
                   currentState = 1;
                   LATAbits.LATA2=1;
               }else if(adc_char>threshold4){// if we go below 5.8V
                    LATAbits.LATA2=0;
               }                
                
                if (adc_char<last_adc){
                    voltage_rised = 1;
                }
                last_adc = adc_char;
            }
        if (counter_Reset >= limit_Reset) {
            counter_Reset = 0;                
            if (voltage_rised==0){ // if there was no increase in voltage during the whole minute
               LATAbits.LATA2 = 0; // Voltage did not recover, turn off to reset
               delayWithWDT(10);          // Initial wait if needed

            }
            voltage_rised = 0;
        }
                

        
    }
}
