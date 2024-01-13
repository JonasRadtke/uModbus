/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */

#include "uartt.h"

void uartInit()
{	

	struct usart_module usart_instance1;
	struct usart_config config_usart;

	//Sercom 1 A1
	usart_get_config_defaults(&config_usart);
	config_usart.baudrate    = MODBUSBAUDRATE;
	config_usart.pinmux_pad0 = PINMUX_UNUSED;
	config_usart.pinmux_pad1 = PINMUX_UNUSED;
	config_usart.pinmux_pad2 = PINMUX_PA24C_SERCOM1_PAD2;
	config_usart.pinmux_pad3 = PINMUX_PA25C_SERCOM1_PAD3;
	while (usart_init(&usart_instance1, SERCOM1, &config_usart) != STATUS_OK){}
	SERCOM1->USART.CTRLA.bit.RXPO = 0x3; // Pad 3
	SERCOM1->USART.CTRLA.bit.TXPO = 0x1; // Pad 2
	usart_enable(&usart_instance1);
	SERCOM1->USART.INTENSET.bit.RXC = 1;
	system_interrupt_enable(SERCOM1_IRQn);
	SERCOM1->USART.CTRLB.bit.TXEN = 1;

	return;
}

// Byte an Pin A1 senden.
void sendByteSercom1 (uint8_t c)
{
	while (!SERCOM1->USART.INTFLAG.bit.TXC && !SERCOM1->USART.INTFLAG.bit.DRE );
	
	SERCOM1->USART.DATA.reg = c;
	return;
}

// Array an Pin A1 senden.
uint32_t sendArraySercom1(uint8_t *x, uint8_t txLength)	// Uart senden eines Strings
{
	uint8_t i = 0;
	while (i < txLength) // Sende bis Ende erreicht
	{
		sendByteSercom1(*x);		// Zeichen senden
		x++;
		i++;					// nächstes zeichen
	}
	while(!SERCOM1->USART.INTFLAG.bit.TXC); // Warte bis SendeBuffer leer
	
	i = SERCOM1->USART.DATA.reg;

	return 1;
}









