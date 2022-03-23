#include "myTimers.h"
#include "IOconfig.h"
#include "encoder.h"
#include "dma.h"
#include "myPWM.h"

static int myCount;
int t;

void initTimer1(unsigned int period)
{

    myCount = 0;
    t = period;
    T1CON = 0;              // ensure Timer 1 is in reset state

    // *** prescaler TCKPS: 11 --> 1:256, 10 --> 1:64, 01 --> 1:8, 00 --> 1:1
    //unsigned int scale = period >> 11;

    if(period<3){ // max Number = 4 194 304 = 2^22 --> period < 158
        period = period * 416.6 * 8 * 8;
        T1CONbits.TCKPS = 0b00;
    }
    else if(period<20){ // max Number = 4 194 304 = 2^22 --> period < 158
        period = period * 416.6 * 8;
        T1CONbits.TCKPS = 0b01;
    }
    else if(period<158){ // max Number = 4 194 304 = 2^22 --> period < 158
        period = period * 416.6;
        T1CONbits.TCKPS = 0b10;
    }
    else if(period<630){ // max Number = 16 777 216 = 2^24 --> period < 630
        period = period * 416.6 / 4;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*2){ // divide period, change LED only every second time
        period = period * 416.6 / 4 / 2;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*4){
        period = period * 416.6 / 4 / 4;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*8){
        period = period * 416.6 / 4 / 8;
        T1CONbits.TCKPS = 0b11;
    }
    else if(period<630*16){
        period = period * 416.6 / 4 / 16;
        T1CONbits.TCKPS = 0b11;
    }




    /*if (period <= 39)
    {
        T1CONbits.TCKPS = 0b00; // FCY divide by 1: tick = 37.5ns
        period = period * 0.0000375;
    }
    else if (period > 39 && period <= 312)
    {
        T1CONbits.TCKPS = 0b01; // FCY divide by 8: tick = 0.3us
        period = period * 0.0003;

    }
    else if (period > 312 && period <= 2500)
    {
        T1CONbits.TCKPS = 0b10; // FCY divide by 64: tick = 2.4us
        period = period * 0.0024;
    }
    else if (period > 2500)
    {
        T1CONbits.TCKPS = 0b11; // FCY divide by 256: tick = 9.6us
        period = period * 0.0096;
    }*/

    //period = period * 416.6;

    //T1CONbits.TCKPS = 0b10; // FCY divide by 64: tick = 2.4us (Tcycle=37.5ns)
    T1CONbits.TCS = 0;      // select internal FCY clock source
    T1CONbits.TGATE = 0;    // gated time accumulation disabled
    TMR1 = 0;
    PR1 = period;           // set Timer 1 period register ()
    IFS0bits.T1IF = 0;      // reset Timer 1 interrupt flag
    IPC0bits.T1IP = 4;      // set Timer1 interrupt priority level to 4
    IEC0bits.T1IE = 1;      // enable Timer 1 interrupt
    T1CONbits.TON = 0;      // leave timer disabled initially
}



void startTimer1(void)
{
    T1CONbits.TON = 1;
}

// this routine found online somewhere, then tweaked
// returns pointer to ASCII string in a static buffer
// TODO: This does not work with long, of course
char *itoa(int value)
{
    static char buffer[12];        // 12 bytes is big enough for an INT32
    int original = value;        // save original value

    int c = sizeof(buffer)-1;

    buffer[c] = 0;                // write trailing null in last byte of buffer

    if (value < 0)                 // if it's negative, note that and take the absolute value
        value = -value;

    do                             // write least significant digit of value that's left
    {
        buffer[--c] = (value % 10) + '0';
        value /= 10;
    } while (value);

    if (original < 0)
        buffer[--c] = '-';

    return &buffer[c];
}


void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
    // Reset timer 1 interrupt flag
    IFS0bits.T1IF = 0;

    char *num = itoa(adcData[0]); // Right
    putsUART1("R: ");
    putsUART1(num);
    putsUART1("\r\nF: ");
    
    num = itoa(adcData[1]); // Front
    putsUART1(num);
    putsUART1("\r\nL: ");
    
    num = itoa(adcData[2]); // Left
    putsUART1(num);
    putsUART1("\r\nE1: ");
    
    
    num = itoa(POS1CNT); // Encoder 1
    putsUART1(num);
    putsUART1("\r\nE2: ");
    
    num = itoa(POS2CNT); // Encoder 2
    putsUART1(num);
    putsUART1("\r\nD1: ");
    
    long distLeft, distRight;
    getEncoderCounts(&distLeft, &distRight);
    
    num = itoa(distLeft); // Distance 1
    putsUART1(num);
    putsUART1("\r\nD2: ");
    
    num = itoa(distRight); // Distance 2
    putsUART1(num);
    putsUART1("\r\n---\r\n");

    LED1 = ~LED1;

    if (BUTTON) {
        LED2 = ~LED2;
    }
//
//    myCount++;
//    /*
//
//    if (LED6 == LEDON){
//        U1TXREG = 'A';
//    }
//    else{
//        U1TXREG = 'Z';
//    }
//    */
//
//    //send all character from A to Z on the serial port
//
//   static char dataOut='A';
//   U1TXREG=dataOut; //0x55;
//   dataOut++;
//   if(dataOut>'Z'){
//       dataOut='A';
//   }
//
//
//   //dataOut = 'A';
//   //U1TXREG=dataOut; //0x55;
//   //dataOut++;
//   //if(dataOut>'Z'){
//   //    dataOut='A';
//   //}
//
//   //char * data = "Hello";
//   //putsUART1(data);
}
