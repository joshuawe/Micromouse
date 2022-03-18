#include "xc.h"
#include "timer1.h"
#include "IOconfig.h"

#define MAX_COUNT 49 //gives us a 0.5s intervall based on a 10ms timer interrupt

//*** sets the timer in a certain mde
void initTimer0(unsigned int period) 
{
    
    
    T1CON = 0;              // ensure Timer 1 is in reset state --> resets the whole register
    T1CONbits.TGATE = 0;    // gated time accumulation disabled
    T1CONbits.TCKPS = 0b10; // FCY divide by 64: tick = 2.4us
    T1CONbits.TCS = 0;      // select internal FCY clock source
    TMR1 = 0;               // *** sets the counter to 0
    PR1 = period;           // set Timer 1 period register ()
    IFS0bits.T1IF = 0;      // reset Timer 1 interrupt flag
    IPC0bits.T1IP = 4;      // set Timer1 interrupt priority level to 4
    IEC0bits.T1IE = 1;      // enable Timer 1 interrupt
    T1CONbits.TON = 0;      // leave timer disabled initially
}

void startTimer0(void)      // *** separate configuration and starting!
{
    T1CONbits.TON = 1; //
 
}


// ***Interrupt function
void __attribute__((__interrupt__, auto_psv)) _T0Interrupt(void)
{
   static int count=0;
   static char dataOut='A';
   IFS0bits.T1IF = 0;
    
    
   IFS0bits.T1IF = 0;           // reset Timer 1 interrupt flag 
   
   /*
   LED4=~LED4; //*** not equal <=> =~
  
   LED6=LEDOFF;
   
     if (count < MAX_COUNT )
    {
        count++;
        if (count%5 == 0)
        {
            LED5=~LED5;
        }
    }
    else  
    {
           LED7=~LED7;
           count=0;
    }
    */
 
   
   //send all character from A to Z on the serial port
   U1TXREG=dataOut; //0x55;
   dataOut++;
   if(dataOut>'Z'){
       dataOut='A';
   }
   
     

}

