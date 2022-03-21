#include "xc.h"
#include "IOconfig.h"
#include "myPWM.h"



//*** sets up the pins in a certain configuration
void setupIO()
{

    int i;
    AD1PCFGL=0xFFFF; //all pins are now digital, by default they are analogue
    //AD1PCFGL = 0xFFDE; // 0b1111 1111 1101 1110; -> pin 0 and 5 are analog pins
                        // AN0 and AN5 are the only inputs !!!CHANGE HERE!!!
                        // AD1 Part Configuration -> by default all bits are 0
                        // Circuit Diagramm: IO6 connects to RB3
                        // Data Sheet RB3 is AN5 (fixed location on the chip)
                        // by default analog functionality
                        // AD Converter does not work


    // LED configuration (set them as output)
    // LED1 is pin 33 (RB4)
    TRISBbits.TRISB4 = 0;
    // LED2 is pin 32 (RA8)
    TRISAbits.TRISA8 = 0;

    // Button configuration (set them as input)
    // BUTTON is pin 26 (RC1)
    TRISCbits.TRISC1 = 1;

    //PIN MAPPING
    // ***** which PIN talks to which PIN ? *****
    // ***** define the multiplexers ! *****

    //before we map, we need to unlock
    __builtin_write_OSCCONL(OSCCON & 0xbf); // clear bit 6 (unlock, they are usually write protected)

    // UART configuration:
    //              RX = PIN43/RB7/RP7
    //              TX = PIN44/RB8/RP8
    // PERIPHERAL receives data from which INPUT
    // Map U1 RX to RP7
    RPINR18bits.U1RXR = 7;
    // OUTPUT PIN receives data from which PERIPHERAL,
    // see table 11-2 in datasheet to check peripheral codes
    // Map RP8 to U1 TX (other way than mapping RX)
    RPOR4bits.RP8R = 0b00011;
    TRISBbits.TRISB8=0;// UART1 TX


    //PERIPHERAL QEA Encoder 1, receives data from RP10
   // RPINR14bits.QEA1R = 10;
    //PERIPHERAL QEB Encoder 1, receives data from RP11
   // RPINR14bits.QEB1R = 11;

    // TODO: Configure ports for encoders
    // Encoder configuration
    // Pin 22 (RB11) Channel A
    // Pin 21 (RB10) Channel B
    //RPINR14bits.QEA1R = 11;
    //RPINR14bits.QEB1R = 10;

    // Motor 1 configuration (left motor)
    TRISCbits.TRISC5 = 0;  // Direction 1, pin 38, RC5
    TRISCbits.TRISC4 = 0;  // Direction 2, pin 37, RC4
    TRISBbits.TRISB12 = 0; // PWM 1, pin 10, PWM1H2, RB12
    LATCbits.LATC5 = 0;  // Pin 38
    LATCbits.LATC4 = 0;  // Pin 37
    LATBbits.LATB12 = 0; // Pin 10
    
    // Motor 2 configuration (right motor)
    TRISBbits.TRISB9 = 0;  // Direction 1, pin 1, RB9
    TRISCbits.TRISC6 = 0;  // Direction 2, pin 2, RC6
    TRISBbits.TRISB14 = 0; // PWM 2, pin 14, PWM1H1, RB14
    LATBbits.LATB9 = 0;  // Pin 1
    LATCbits.LATC6 = 0;  // Pin 2
    LATBbits.LATB14 = 0; // Pin 14

    // TODO: Check this comment
    //Setting PWM (pin 21 = PWM1H3, pin 22 = PWM1L3, pin 23 = H2, pin 24 = L2, pin25 = H1, pin26 = L1)
    //setupPWM(1,0,0,0);
    
    //after mapping we lock again
     __builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS registers (lock again!)

    for (i = 0; i < 30000; i++); // short dirty delay for changes to take effect,

}
