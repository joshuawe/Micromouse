#include "xc.h"
#include "IOconfig.h"
#include "myPWM.h"



//*** sets up the pins in a certain configuration
void setupIO()
{

    int i;
    //AD1PCFGL=0xFFFF; //all pins are now digital, by default they are analogue
    AD1PCFGL = 0xFFDE; // 0b1111 1111 1101 1110; -> pin 0 and 5 are analog pins
                        // AN0 and AN5 are the only inputs !!!CHANGE HERE!!!
                        // AD1 Part Configuration -> by default all bits are 0
                        // Circuit Diagramm: IO6 connects to RB3
                        // Data Sheet RB3 is AN5 (fixed location on the chip)
                        // by default analog functionality
                        // AD Converter does not work
    
    
    // set LEDs as output
    TRISBbits.TRISB15 = 0;
    TRISBbits.TRISB14 = 0;
    TRISBbits.TRISB13 = 0;
    TRISBbits.TRISB12 = 0;
    
    TRISBbits.TRISB8=0;// UART1 TX

    //PIN MAPPING
    // ***** which PIN talks to which PIN ? *****
    // ***** define the multiplexers ! *****
    
    //before we map, we need to unlock
    __builtin_write_OSCCONL(OSCCON & 0xbf); // clear bit 6 (unlock, they are usually write protected)
    
    // ***** defines the input pin *****
    // PERIPHERAL receives data from which INPUT  
    RPINR18bits.U1RXR = 9; //mapped to RP9 is U1 RX, CHANGE THIS

    
    //PERIPHERAL QEA Encoder 1, receives data from RP10
   // RPINR14bits.QEA1R = 10; 
    //PERIPHERAL QEB Encoder 1, receives data from RP11
   // RPINR14bits.QEB1R = 11;
    
    
    // ***** defines the output pin *****
    //OUTPUT PIN receives data from which PERIPHERAL, 
    //see table 11-2 in datasheet to check peripheral codes 
    RPOR4bits.RP8R = 0b00011; //output bin RP2 gets data from peripheral U1 TX 

   
    //after mapping we lock again
     __builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS registers (lock again!)
     
    for (i = 0; i < 30000; i++); // short dirty delay for changes to take effect,

    
}

void setupIO_Motor(){
  
    int i;
    AD1PCFGL=0xFFFF; //all pins are now digital, by default they are analog
    //AD1PCFGL = 0xFFDE; // 0b1111 1111 1101 1110; -> pin 0 and 5 are analog pins
                        // AN0 and AN5 are the only inputs !!!CHANGE HERE!!!
                        // AD1 Part Configuration -> by default all bits are 0
                        // Circuit Diagramm: IO6 connects to RB3
                        // Data Sheet RB3 is AN5 (fixed location on the chip)
                        // by default analog functionality
                        // AD Converter does not work
    
    
    //PIN MAPPING
    // ***** which PIN talks to which PIN ? *****
    // ***** define the multiplexers ! *****
    
    //before we map, we need to unlock
    __builtin_write_OSCCONL(OSCCON & 0xbf); // clear bit 6 (unlock, they are usually write protected)
    
 
    
    //PERIPHERAL QEA Encoder 1, receives data from RP10
   // RPINR14bits.QEA1R = 10; 
    //PERIPHERAL QEB Encoder 1, receives data from RP11
   // RPINR14bits.QEB1R = 11;
    
 
    //Setting input pins for encoder
    RPINR14bits.QEA1R = 14; // pin RP14 = Pin 25 assigned to register 14A as input
    RPINR14bits.QEB1R = 15; // pin RP15 = Pin 26 assigned to register 14B as input
    
    //Setting power and ground pins
    TRISBbits.TRISB13 = 0; // output pin 24
    TRISBbits.TRISB12 = 0; // output pin 23
    TRISBbits.TRISB15 = 0; // output pin 25 --> nachschauen!!!
    LATBbits.LATB13 = 1; // Power pin 24
    LATBbits.LATB12 = 0; // Ground pin 23
    LATBbits.LATB15 = 0; // Ground pin 25 --> nachschauen!!!

    //Setting PWM (pin 21 = PWM1H3, pin 22 = PWM1L3, pin 23 = H2, pin 24 = L2, pin25 = H1, pin26 = L1)
    
    
    setupPWM(0.8,0,0,1);  
    
    //after mapping we lock again
     __builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS registers (lock again!)
     
    for (i = 0; i < 30000; i++); // short dirty delay for changes to take effect,

      
    
    

}
