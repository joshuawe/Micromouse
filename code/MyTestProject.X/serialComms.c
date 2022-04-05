#include <math.h>
#include "xc.h"
#include "IOconfig.h"
#include "serialComms.h"
#include "ICC.h"

/*
*	set-up the serial port
*   here we aim to achieve a data transfer rate of 57.6 kbit/s,
*   based on Fcycle=26.6666Mhz 
*   BaudRate=Fcycle/(16*(BRG+1))
*   ==> BRG=Fcy/(16*BaudRate) - 1 = 26.666Mhz/(16*57600) - 1 = 28.23
*   ==> choose 28 ==> BaudRate= 57.474  kbit/s, which is ~ 1% off.
 * 
 * for standard communication speed of 9600 kbit/s
 * choose 173 (factor 6)
*/
void setupUART1(void)
{
	U1MODEbits.UARTEN=0; //switch the uart off during set-up
    
    
	U1BRG= 173; // baud rate register
	U1MODEbits.LPBACK=0; // in loopback mode for test! TODO: set to no loop-back (=0) after test 
	
	U1MODEbits.WAKE=0; //do not wake up on serial port activity

	U1MODEbits.ABAUD=0; //no auto baud rate detection
	U1MODEbits.PDSEL=0; //select 8 bits date, no parity
	U1MODEbits.STSEL=0; //one stop bit
    U1MODEbits.BRGH = 0; // No High Speed Mode


	IFS0bits.U1RXIF=0; //reset the receive interrupt flag
	IFS0bits.U1TXIF=0; //reset the transmission interrupt flag
    
	IPC2bits.U1RXIP=3; //set the RX interrupt priority
	IPC3bits.U1TXIP=5; //set the TX interrupt priority

	U1STAbits.URXISEL=0; //generate a receive interrupt as soon as a character has arrived
	U1STAbits.UTXEN=1; //enable the transmission of data

	IEC0bits.U1RXIE=1; //enable the receive interrupt
	IEC0bits.U1TXIE=0; //disable the transmit interrupt

	//FINALLY, 
	U1MODEbits.UARTEN=1; //switch the uart on

  	U1STAbits.UTXEN=1; //enable transmission
	
    
//   	U1MODE = 0x8000; /* Reset UART to 8-n-1, alt pins, and enable */
//	U1STA  = 0x0440; /* Reset status register and enable TX & RX*/

	
	
}

#define STATE_AWAIT_SIG1 0 // Waiting for M
#define STATE_AWAIT_SIG2 1 // Waiting for M
#define STATE_AWAIT_SIG3 2 // Waiting for M
#define STATE_AWAIT_SIG4 3 // Waiting for J
#define STATE_AWAIT_COL  4 // Waiting for :
#define STATE_READ_CMD   5
#define STATE_READ_PARAM 6
#define CMD_MAX_LEN 16

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{	
	unsigned char rxData; // a local buffer to copy the data into

    static unsigned short state = STATE_AWAIT_SIG1;
    static char cmd[CMD_MAX_LEN];
    static char param[CMD_MAX_LEN];
    static unsigned int cmd_index = 0;
    static unsigned int param_index = 0;
    
	/**Set the UART2 receiving interrupt flag to zero*/
	IFS0bits.U1RXIF=0;

    LED2 =~LED2;
    
	//we should now read out the data
	rxData=U1RXREG;
    
    switch(state) {
        case STATE_AWAIT_SIG1:
            if (rxData == 'M') state =  STATE_AWAIT_SIG2;
            else state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_SIG2:
            if (rxData == 'M') state =  STATE_AWAIT_SIG3;
            else state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_SIG3:
            if (rxData == 'M') state =  STATE_AWAIT_SIG4;
            else state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_SIG4:
            if (rxData == 'J') state =  STATE_AWAIT_COL;
            else state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_COL:
            if (rxData == ':') {
                state =  STATE_READ_CMD;
                cmd_index = 0;
            }
            else state =  STATE_AWAIT_SIG1;
            break;
        case STATE_READ_CMD:
            if (rxData == ':') {
                state = STATE_READ_PARAM;
                param_index = 0;
                cmd[cmd_index] = '\0';
            } else if(cmd_index < CMD_MAX_LEN) {
                cmd[cmd_index] = rxData;
                cmd_index += 1;
            } else state =  STATE_AWAIT_SIG1;
            break;
        case STATE_READ_PARAM:
            if (rxData == '\r') {
            } else if (rxData == '\n') {
                // We are done!
                state = STATE_AWAIT_SIG1;
                param[param_index] = '\0';
                execute_bt_command(cmd, param);
            } else if(param_index < CMD_MAX_LEN) {
                param[param_index] = rxData;
                param_index += 1;
            } else state =  STATE_AWAIT_SIG1;
            break;
    }
    
    //and copy it back out to UART
    // For our micromouse: Do not output all input back again. Otherwise the
    // bluetooth module can get messed up.
    //U1TXREG=rxData;
        //wait until the character is gone...

	//we should also clear the overflow bit if it has been set (i.e. if we were to slow to read out the fifo)
	U1STAbits.OERR=0; //we reset it all the time
	//some notes on this from the data sheet
	/*
	If the FIFO is full (four characters) and a fifth character is fully received into the UxRSR register,
	the overrun error bit, OERR (UxSTA<1>), will be set. The word in UxRSR will be kept, but further
	transfers to the receive FIFO are inhibited as long as the OERR bit is set. The user must clear
	the OERR bit in software to allow further data to be received.
	If it is desired to keep the data received prior to the overrun, the user should first read all five
	characters, then clear the OERR bit. If the five characters can be discarded, the user can simply
	clear the OERR bit. This effectively resets the receive FIFO and all prior received data is lost.

	The data in the receive FIFO should be read prior to clearing the OERR bit. The
	FIFO is reset when OERR is cleared, which causes all data in the buffer to be lost.
	*/

}

int count = 0;
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{	
	//unsigned int rxData; // a local buffer to copy the data into
   // long i;
	/*Set the UART2 receiving interrupt flag to zero*/
	IFS0bits.U1TXIF=0;
    //LED7=LEDON;//;
    
    if(count%5==0){
        //LED4=~LED4;
    }
    
}


/***************************************************************************
* Function Name     : putsUART1                                            *
* Description       : This function puts the data string to be transmitted *
*                     into the transmit buffer (till NULL character)       *
* Parameters        : unsigned int * address of the string buffer to be    *  
*                     transmitted                                          *
* Return Value      : None                                                 *
***************************************************************************/

void putsUART1(char *buffer)
{
    char * temp_ptr = (char *) buffer;
    
    //LED7=LEDON;
    
    /* transmit till NULL character is encountered */

    if(U1MODEbits.PDSEL == 3)        /* check if TX is 8bits or 9bits */
    {
        while(*buffer != '\0') 
        {
            while(U1STAbits.UTXBF); /* wait if the buffer is full */
            U1TXREG = *buffer++;    /* transfer data word to TX reg */
        }
    }
    else
    {
        while(*temp_ptr != '\0')
        {
            while(U1STAbits.UTXBF);  /* wait if the buffer is full */
            U1TXREG = *temp_ptr++;   /* transfer data byte to TX reg */
        }
    }
}
