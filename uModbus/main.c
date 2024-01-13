/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */

#include <asf.h>
#include "main.h"

	volatile uint32_t ticks = 0; // Time since Start (ms)
	mbDataMapping modbusData;
	mbComm modbusComm;

int main (void)
{
	system_init();
	SysTick_Config(48000000 / 1000); //Systemtimer config

	uartInit();

	// Test Data
	modbusData.inputRegisters[0] = 125;
	modbusData.inputRegisters[1] = 50000;
	modbusData.holdingRegisters[0] = 500;
	modbusData.holdingRegisters[1] = 60000;
	modbusData.coils[0] = 0xDC;
	modbusData.coils[1] = 0x66;

	while (1)
	{
		modbusSlaveCyclic(&modbusComm ,&modbusData); // Cyclic modbusSlave call
	}
}

void SysTick_Handler(){
	ticks++;
	MBTimer();
}




