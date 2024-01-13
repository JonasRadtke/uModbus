/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */


#ifndef UARTT_H_
#define UARTT_H_

#include "asf.h"
#include "modbusSlave.h"
#include <stdint.h>

void uartInit(void);

void sendByteSercom1(uint8_t);
uint32_t sendArraySercom1(uint8_t *, uint8_t);


#endif /* UART_H_ */