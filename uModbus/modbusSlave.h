/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */


#ifndef MODBUSSLAVE_H_
#define MODBUSSLAVE_H_

#include <stdint.h>
#include "modbusConf.h"

typedef struct
{
	#if MAX_DISCRETE_INPUTS > 0
		#if MAX_DISCRETE_INPUTS%8 > 0
			uint8_t discreteInputs[(MAX_DISCRETE_INPUTS/8)+1];
		#else
			uint8_t discreteInputs[(MAX_DISCRETE_INPUTS/8)];
		#endif
	#endif
	#if MAX_COILS > 0
		#if MAX_COILS%8 > 0
			uint8_t coils[(MAX_COILS/8)+1];	
		#else
			uint8_t coils[(MAX_COILS/8)];
		#endif
	#endif				
	#if MAX_INPUT_REGISTERS > 0	
		uint16_t inputRegisters[MAX_INPUT_REGISTERS];
	#endif
	#if MAX_HOLDING_REGISTERS > 0	
		uint16_t holdingRegisters[MAX_HOLDING_REGISTERS];
	#endif
} mbDataMapping;

typedef struct
{
	uint16_t memoryadress;
	uint16_t memorylength;
	uint8_t txBuffer[BUFFERLENGTH];
	uint32_t txLength;
volatile uint8_t rxBuffer[BUFFERLENGTH];
volatile uint32_t rxLength;
volatile uint32_t rxActive;
volatile uint32_t rxDone;
} mbComm;

void modbusSlaveCyclic(mbComm * , mbDataMapping *);

uint16_t crc16(uint8_t * , uint8_t);
void exceptionCode(mbComm *, uint16_t, uint8_t);
void sendSlaveResponse(mbComm *);
void receiveModbusByte(mbComm *, uint8_t);
uint32_t MBsendMessage(uint8_t *, uint8_t);
void clearTxBuffer(mbComm *);
uint32_t MBcheckTimer(void);
void MBTimer (void);

#endif /* MODBUSSLAVE_H_ */