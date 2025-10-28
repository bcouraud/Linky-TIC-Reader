// For Vref (supply) =5V:
//ADC = 24 --> 0.47V 
// ADC = 48 --> 0.95 
// ADC = 100 --> 1.96V
// ADC = 150 --> 2.98V
// ADC = 200 --> 3.96V
// ADC = 250 --> 4.98V







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
unsigned int counter_RisingVoltage = 0;
unsigned int counter_Reset = 0;
unsigned int limit_ADC = 1; //1;          // ~1 cycles of 1s
unsigned int limit_RisingVoltage = 4; //1;          // ~4 cycles of 1s
unsigned int limit_Reset = 35;          // ~60 cycles of 1s ~ 1 minute  (8 cycles = 14s)
unsigned char threshold3 = 146 ; //590; //147 * 1023 / 255 ; 147;   // 6.1V Voltage must rise to this
unsigned char threshold4 = 151 ; //605; //151 * 1023 / 255 ? 605     ; 151;   // 5.7V Voltage low threshold (from zener)
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

    return (ADRESH);//return ((ADRESH << 8) | ADRESL); // Combine high + low byte
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
    ADCON1bits.ADFM = 0; // left-justified (8-bits)1;  // Right justified (10-bit)
    ADCON0bits.CHS = 0b0000;     // RA0
    APFCONbits.P1BSEL = 0;
    TRISAbits.TRISA3 = 0;    // Make RA3 an output

    // INIT UART
            APFCONbits.RXDTSEL = 1; //Select RX on RA5 (instead of RA0)
            APFCONbits.TXCKSEL = 1; //Select TX on RA4 (instead of RA1)
            APFCONbits.P1BSEL = 0 ;
            TRISAbits.TRISA4 = 0;   // RA4 = TX= output
            TRISAbits.TRISA5 = 1;   // RA5 = RX = INPUT
            //Configuration Data Rate
            BAUDCONbits.BRG16 = 1;//0;
            SPBRG = 25;   // = Fosc/(16*DataRate)-1;  Fosc = 1MHz et DataRate = 9600
            //SPBRGH=25>>8;
            //SPBRGL = 25&0xFF;
            TXSTA = 0b00110110; // Asynchron TX config, sur 8bits,
            TXSTAbits.BRGH = 1;
            TXSTAbits.SYNC = 0;
            RCSTA = 0b10010000; // Asynchron RX config, sur 8 bits
    
    
    
    
    
    
    
    
    
    // --- Initial Power ON ---
    LATAbits.LATA1 = 1;         // Start lecture by putting power on resistance and diode
    last_adc = ADC_Lecture();
    LATAbits.LATA1 = 0;         // 

    delayWithWDT(1);          // Initial wait if needed

    while (1) {
        counter_ADC++;
        counter_Reset++;
        counter_RisingVoltage++;
            //delayWithWDT(2);          // Initial wait if needed

        LATAbits.LATA1 = 0;   // Ensure pin powers off on resistance and diode
        SLEEP();                // Sleep ~1s (WDT)
        NOP();

        if (counter_ADC >= limit_ADC) {
            counter_ADC = 0;
          //  LATAbits.LATA2 ^= 1;

                LATAbits.LATA1 =1;  // Start lecture by putting power on resistance and diode
                adc_char = ADC_Lecture();
                LATAbits.LATA1 =0;
                
                // we send the result through UART 
                   // TXREG = 00;
                   // while(!TXSTAbits.TRMT);
                    //TXREG = adc_char;
                    //while(!TXSTAbits.TRMT); // we send the result through UART 
                
                
                
                
                
               if (adc_char<threshold3){ // if we reached 6V   (1.4V when supplied at 5V in a test (without zener diode))
                   currentState = 1;
                   LATAbits.LATA2=1;
               }else if(adc_char>threshold4){// if we go below 5.8V
                    LATAbits.LATA2=0;
               }                
                
            }
        if (counter_RisingVoltage >= limit_RisingVoltage) {
            counter_RisingVoltage = 0;
                if (adc_char<last_adc-2){
                    voltage_rised = 1;
                   // TXREG = 255;
                   // while(!TXSTAbits.TRMT); // we send the result through UART 
                }
            
            last_adc = adc_char;
            
        }
        
        
        if (counter_Reset >= limit_Reset) {
            counter_Reset = 0;   
                    //            TXREG = 19;
                    //while(!TXSTAbits.TRMT); // we send the result through UART 


            if (voltage_rised==0){ // if there was no increase in voltage during the whole minute
               LATAbits.LATA2 = 0; // Voltage did not recover, turn off to reset
               delayWithWDT(10);          // Initial wait if needed
                   // TXREG = 255;
                    //while(!TXSTAbits.TRMT); // we send the result through UART 
 

            }
            voltage_rised = 0;
        }

    }
}
