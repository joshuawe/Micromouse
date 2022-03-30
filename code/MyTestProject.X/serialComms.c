#include <math.h>
#include "xc.h"
#include "IOconfig.h"
#include "serialComms.h"

// signature for the protocoll for communication with PC
const char SIGNATURE[] = "MMMJ";
const int PRECISION = 4;   // logging precision for double/float variables


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

unsigned short cmd_state = STATE_AWAIT_SIG1;
char cmd_cmd[CMD_MAX_LEN];
char cmd_param[CMD_MAX_LEN];
unsigned int cmd_index = 0;
unsigned int param_index = 0;


void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{	
	unsigned char rxData; // a local buffer to copy the data into

	/**Set the UART2 receiving interrupt flag to zero*/
	IFS0bits.U1RXIF=0;

    LED2 =~LED2;
    
	//we should now read out the data
	rxData=U1RXREG;
    
    switch(cmd_state) {
        case STATE_AWAIT_SIG1:
            if (rxData == 'M') cmd_state =  STATE_AWAIT_SIG2;
            else cmd_state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_SIG2:
            if (rxData == 'M') cmd_state =  STATE_AWAIT_SIG3;
            else cmd_state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_SIG3:
            if (rxData == 'M') cmd_state =  STATE_AWAIT_SIG4;
            else cmd_state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_SIG4:
            if (rxData == 'J') cmd_state =  STATE_AWAIT_COL;
            else cmd_state =  STATE_AWAIT_SIG1;
            break;
        case STATE_AWAIT_COL:
            if (rxData == ':') {
                cmd_state =  STATE_READ_CMD;
                cmd_index = 0;
            }
            else cmd_state =  STATE_AWAIT_SIG1;
            break;
        case STATE_READ_CMD:
            if (rxData == ':') {
                cmd_state = STATE_READ_PARAM;
                param_index = 0;
                cmd_cmd[cmd_index] = '\0';
            } else if(cmd_index < CMD_MAX_LEN) {
                cmd_cmd[cmd_index] = rxData;
                cmd_index += 1;
            } else cmd_state =  STATE_AWAIT_SIG1;
            break;
        case STATE_READ_PARAM:
            if (rxData == '\r') {
            } else if (rxData == '\n') {
                // We are done!
                cmd_state = STATE_AWAIT_SIG1;
                cmd_param[param_index] = '\0';
            } else if(param_index < CMD_MAX_LEN) {
                cmd_param[param_index] = rxData;
                param_index += 1;
            } else cmd_state =  STATE_AWAIT_SIG1;
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

void logInt(char * name, int num)
{
    char *str = itoa(num);
    putsUART1(name);
    putsUART1(": ");
    putsUART1(str);
    putsUART1("\t");
}

// Use this to send float/double variable with communication protocol
void sendFloat(char * name, float num)
{
    int endpoint = PRECISION;
    char str[10];
    ftoa(num, str, endpoint);
    putsUART1("\n");
    putsUART1(SIGNATURE);
    putsUART1(":");
    putsUART1(name);
    putsUART1(":");
    putsUART1(str);
    putsUART1("\n\r");
}

// Use this to send int variable with communication protocol
void sendInt(char * name, int num)
{
    char *str = itoa(num);
    putsUART1("\n");
    putsUART1(SIGNATURE);
    putsUART1(":");
    putsUART1(name);
    putsUART1(":");
    putsUART1(str);
    putsUART1("\n\r");
}


/* 
 * The next few functions for ftoa(_) implementation are inspired by 
 * https://www.geeksforgeeks.org/convert-floating-point-number-string/)
*/
// Reverses a string 'str' of length 'len'
void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
  
// Converts a given integer x to string str[]. 
// d is the number of digits required in the output. 
// If d is more than the number of digits in x, 
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
  
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
  
    reverse(str, i);
    str[i] = '\0';
    return i;
}
  
// Converts avoid ftoa(float n, char* res, int afterpoint) floating-point/double number to a string.
void ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
  
    // Extract floating part
    float fpart = n - (float)ipart;
  
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
  
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
  
        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter 
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
  
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}