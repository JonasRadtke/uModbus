/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */


#ifndef MODBUSCONF_H_
#define MODBUSCONF_H_

#define MODBUSBAUDRATE						19200
#define SLAVEID								1				// 1 - 247

// Bufferlength, modbus max message is 256. for few data you can make it smaller.
#define BUFFERLENGTH						256				// max 256 needed for modbus standard

// Starting adress and number of discrete inputs
#define DISCRETE_INPUT_START_ADRESS			0				// Single Bit, Read Only
#define MAX_DISCRETE_INPUTS					8				// 1 Input = 1 bit Max 65536

// Starting adress and number of coils
#define COILS_START_ADRESS					0				// Single Bit, Read-Write
#define MAX_COILS							8				// Max 65536

// Starting adress and number of input registers
#define INPUT_REGISTERS_START_ADRESS		0				// 16bit word, Read Only
#define MAX_INPUT_REGISTERS                 8				// Max 65536

// Starting adress and number of holding registers
#define HOLDING_REGISTERS_START_ADRESS		0				// 16bit word, Read-Write
#define MAX_HOLDING_REGISTERS               8				// Max 65536

#define OVERLAYING							1

#endif /* MODBUSCONF_H_ */