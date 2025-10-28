
//#include <pic12lf1840.h>
#include <pic12lf1822.h>

// For Vref (supply) =5V:
//ADC = 24 --> 0.47V 
// ADC = 48 --> 0.95 
// ADC = 100 --> 1.96V
// ADC = 150 --> 2.98V
// ADC = 200 --> 3.96V
// ADC = 250 --> 4.98V







// CONFIG1
#pragma config FOSC = INTOSC    
#pragma config WDTE = OFF       
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

// ----------- Global Variables -----------
volatile unsigned char triggerFlag = 0;
volatile unsigned char t1_overflows = 0;




#include <xc.h>
#include <math.h>
//#include <stdbool.h>

// ----------- Global Variables -----------
unsigned int counter_ADC = 0;
unsigned int counter_RisingVoltage = 0;
unsigned int counter_Reset = 0;
unsigned int limit_ADC = 1; //1;          // ~1 cycles of 1s
unsigned int limit_RisingVoltage = 4; //1;          // ~4 cycles of 1s
unsigned int minVoltage = 255; //1;          // ~4 cycles of 1s
unsigned int maxVoltage = 0; //1;          // ~4 cycles of 1s
unsigned int limit_Reset = 15;//35;          // ~60 cycles of 1s ~ 1 minute  (8 cycles = 14s)
unsigned int threshold1 = 240;// 251; //251 = 6.50  //245 = 6.36V for start up; //590; //147 * 1023 / 255 ; 147;   // 6.1V Voltage must rise to this
unsigned int threshold2 = 190; //228; //235 = 6.09   // 240 ; //605; //151 * 1023 / 255 ? 605     ; 151;   // 5.7V Voltage low threshold (from zener)
unsigned int adc_char = 0;
unsigned int last_adc = 0;
unsigned int voltage_rised = 0;
unsigned int currentState = 1;
unsigned int ESPState = 0;
unsigned int limit_counter_ESP = 10;
unsigned int counter_ESP = 0;

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
        __delay_ms(5);  // Keep < WDT timeout (~1s)
    }
}


// ----------- ISR -----------
void __interrupt() isr(void) {
    // IOC: Falling edge on RA4
    if (IOCIF && IOCAFbits.IOCAF4) {
        IOCAFbits.IOCAF4 = 0;   // clear flag
        __delay_ms(500);        // Wait for ESP to settle

                if (PORTAbits.RA4 == 1) {
                    ESPState =1;
                }

        if (PORTAbits.RA4 == 0) {
            LATAbits.LATA2 = 1;   // Turn RA2 OFF
            ESPState = 0;

            currentState = 0; // we prevent the ADC threshold to turn ON the ESP, as we must wait 20sec.
            // Start Timer1 once
            t1_overflows = 0;
            TMR1 = 0;             
            PIR1bits.TMR1IF = 0;
            PIE1bits.TMR1IE = 1;  // enable Timer1 interrupt
            T1CONbits.TMR1ON = 1; // start Timer1
        }
    }

    // Timer1 overflow: ~2.097s per overflow
    if (PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0;   // clear flag
        t1_overflows++;

        if (t1_overflows >= 7) {  // ~21 seconds
            adc_char = ADC_Lecture();
            if(adc_char >= threshold2){
                LATAbits.LATA2 = 0;    // Turn RA2 ON
                __delay_ms(500);        // Wait for ESP to settle


            }
            T1CONbits.TMR1ON = 0;  // stop Timer1
            PIE1bits.TMR1IE = 0;   // disable interrupt
            currentState = 1;
        }
    }
}


// ----------- Main -----------
void main(void) {
    OSCCON = 0b01011010;         // Internal OSC, 1MHz
    OSCCONbits.SCS1 = 1;
    // --- WDT + Oscillator setup ---
    CLRWDT();

    
    
    // --- IO setup ---
    TRISA = 0b00100001;          // RA5=input (RX), RA0=input (ADC), others output
    ANSELA = 0x01;               // RA0 analog, others digital
    LATA = 0x00;
    LATAbits.LATA2 = 0;          // Ensure RA2 low initially
    OPTION_REGbits.nWPUEN = 0; // enable weak pull-ups globally
    WPUAbits.WPUA4 = 1;        // pull-up on RA4

    // --- WDT + Oscillator setup ---
    CLRWDT();
    ANSELAbits.ANSA0 = 1;        // RA0 analog
    ADCON1bits.ADCS = 0b000;
    ADCON1bits.ADPREF = 0b00;    
    ADCON1bits.ADFM = 0; // left-justified (8-bits)1;  // Right justified (10-bit)
    ADCON0bits.CHS = 0b0000;     // RA0
    APFCONbits.P1BSEL = 0;
    TRISAbits.TRISA3 = 0;    // Make RA3 an output
    TRISAbits.TRISA2 = 0;    // Make RA2 an output
    LATAbits.LATA2=1;  // set it high so the ESP supply is turned off
    TRISAbits.TRISA4 = 1;   // Configure RA4 as input
    




    // --- Interrupt-on-change setup (RA4) ---
    IOCAPbits.IOCAP4 = 0;        // Disable rising edge
    IOCANbits.IOCAN4 = 1;        // Enable falling edge
    IOCAFbits.IOCAF4 = 0;        // Clear flag
    IOCIF = 0;                   // Clear IOC interrupt flag
    IOCIE = 1;                   // Enable IOC interrupt


    T1CONbits.TMR1CS = 0;     // Clock = Fosc/4
    T1CONbits.T1CKPS = 0b11;  // Prescaler = 1:8
    T1CONbits.TMR1ON = 0;     // Initially off
    PIE1bits.TMR1IE = 0;      
    PIR1bits.TMR1IF = 0;

    // --- Interrupts ---
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;


    // --- Main loop ---
    while (1) {
        
                counter_ADC++;

      if (counter_ADC >= limit_ADC && currentState==1) {
            counter_ADC = 0;
            adc_char = ADC_Lecture();
            
               if (adc_char>threshold1){ // if we reached 6V 
                    LATAbits.LATA2=0; 

                    
                    
                    
                    
                    
                    
                    
                    
                    //             __delay_ms(100);  
              //  while(ESPState==0 && PORTAbits.RA4 == 0){
               //     LATAbits.LATA2 = 1;    // Turn RA2 OFF
               //     __delay_ms(100);  
              //        LATAbits.LATA2 = 0;    // Turn RA2 ON
              //      __delay_ms(100);  
             //   }            
                   
               }else if(adc_char<threshold2){// if we go below 5.8V
                    LATAbits.LATA2=1;
                       ESPState = 0;
                   __delay_ms(500);        // Wait for ESP to settle

               }else{  // if we are in a state where the ESP should run, and voltage above threshold2
                       __delay_ms(500);        // Wait for ESP to settle

                            if (PORTAbits.RA4 == 0) { // if the ESP is not pulling up RA4
                                LATAbits.LATA2 = 1;   // Turn RA2 OFF (we shut down the ESP for a few seconds)
                                ESPState = 0;
                                currentState = 0; // we prevent the ADC threshold to turn ON the ESP, as we must wait 20sec.
                              __delay_ms(500);        // Wait for ESP to settle
                                LATAbits.LATA2 = 0;   // Turn RA2 ON (we reset the ESP)
                                ESPState = 1;
                                currentState = 1; // we update the current state

                                // reStart Timer1 
                                t1_overflows = 0;
                                TMR1 = 0;             
                                PIR1bits.TMR1IF = 0;
                                PIE1bits.TMR1IE = 1;  // enable Timer1 interrupt
                                T1CONbits.TMR1ON = 1; // start Timer1
                            }                   
                   
               }               
  
            }                
       /* if (triggerFlag) {
            triggerFlag = 0;

            LATAbits.LATA2 = 0;   // Turn RA2 ON

            // Hold for ~20s using WDT cycles
            for (uint8_t i = 0; i < 5; i++) {
                CLRWDT();        // Reset WDT
                SLEEP();         // Sleep, wakeup by WDT
            }

            LATAbits.LATA2 = 1;   // Turn RA2 OFF
        }*/
    }
}

