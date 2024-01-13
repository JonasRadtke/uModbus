/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */

#include "modbusSlave.h"
#include "uartt.h"

static volatile uint32_t Timer1, Timer2;
extern mbComm modbusComm;

// Uart Interrupt call receiveModbusByte in here to give the new byte to the stack
void SERCOM1_Handler(){
	uint8_t recvByte;
	// new byte received
	if (SERCOM1->USART.INTFLAG.bit.RXC)
	{
		recvByte = SERCOM1->USART.DATA.reg; 

		receiveModbusByte(&modbusComm, recvByte);
		Timer1 = 2; // timeout
	}
	return;
}

// Send the response
uint32_t MBsendMessage(uint8_t *x, uint8_t txLength){
	// Put Code in her to send a byte array with given length
	sendArraySercom1(x,txLength);
	return 0;
}

// customize this code to return a 1 when timer is zero
uint32_t MBcheckTimer(void){
	if (!Timer1)
	{
		// Return 1 when rx frame complete
		return 1;
	}
	return 0;
}

// call this function in a 1khz timer (systemtimer or timer/counter)
void MBTimer (void)
{
	uint32_t x;
	
	x = Timer1;						// 1kHz decrement timer stopped at 0
	if (x) Timer1 = --x;
	x = Timer2;
	if (x) Timer2 = --x;
}

