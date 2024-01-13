/*
 * Copyright © Jonas Radtke
 *
 * License GPL-3.0-or-later
 */

#include "modbusSlave.h"

#define READ_COILS					0x01
#define READ_DISCRETE_INPUTS		0x02
#define READ_HOLD_REGISTERS			0x03
#define READ_INPUT_REGISTERS		0x04
#define WRITE_SINGLE_COIL			0x05
#define WRITE_SINGLE_REGISTER		0x06
#define WRITE_MULTIPLE_COILS		0x0F
#define WRITE_MULTIPLE_REGISTERS	0x10
#define ERROR					    0xFFFF

#define ILLEGAL_FUNCTION			0x01
#define ILLEGAL_DATA_ADDRESS		0x02
#define ILLEGAL_DATA_VALUE			0x03



void modbusSlaveCyclic(mbComm *com, mbDataMapping *data)
{
	uint32_t i;
    uint16_t functionCode = 0;
	uint8_t memoryadress = 0;
	uint8_t memorylength = 0;
	uint16_t crc;
	uint16_t crcMessage;
	uint16_t actualBit;
	uint8_t shift;

	// Prüfen ob letztes Zeichen empfangen wurde. Empfang Aktiv + Abgelaufener Timer
	if (com->rxActive && (MBcheckTimer()))
	{
		com->rxActive = 0;
		com->rxDone = 1;
		
		// CRC Prüfen, letzte beiden Bytes im Frame
		crc = crc16(&com->rxBuffer[0], com->rxLength-2);
		crcMessage = (com->rxBuffer[com->rxLength - 1] << 8) | (com->rxBuffer[com->rxLength - 2] & 0xFF);
		
		if (crc != crcMessage)
		{
			functionCode = ERROR; // CRC Falsch oder Daten kaputt -> keine Antwort
		}
		else if (com->rxBuffer[0] == SLAVEID) // Functions Code, Speicheradresse und Speicherlänge exthrahieren
		{
			functionCode = (uint16_t)com->rxBuffer[1];
		}
		else{
			functionCode = ERROR; // Falsche SlaveID -> keine Antwort
		}
		com->rxDone = 1;
	}
	
	if (functionCode)
	{
		switch (functionCode)
		{
			#if MAX_COILS > 0
			case READ_COILS: // Code 1 Read Coils
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				memorylength = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
				shift = memoryadress % 8;
			
				// 0x0001 <= Quantity of Outputs <= 0x07B0
				if (!((memorylength >= 0x0001) && (memorylength <= 0x07D0))) exceptionCode(com, READ_COILS, ILLEGAL_DATA_VALUE);
				// Starting Address == OK
				else if (memoryadress >= (COILS_START_ADRESS + MAX_COILS)) exceptionCode(com, READ_COILS, ILLEGAL_DATA_ADDRESS);
				// Starting Address + Quantity of Outputs == OK
				else if (memoryadress + memorylength > (COILS_START_ADRESS + MAX_COILS)) exceptionCode(com, READ_COILS, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = READ_COILS;
					if (memorylength%8) com->txBuffer[2] = (memorylength/8)+1;
					else com->txBuffer[2] = memorylength/8;

					actualBit = 0;
					clearTxBuffer(com);
					// Read Coils and create response
					for (i=0; i < memorylength; i++)
					{
						if ((data->coils[(shift + actualBit)/8] >> (shift + actualBit)%8) & 0x01)
						{
							com->txBuffer[3 + actualBit/8] = (0x01 << actualBit%8) | (com->txBuffer[3 + actualBit/8] & (~(0x01 << actualBit%8)));
						}
						else
						{
							com->txBuffer[3 + actualBit/8] = (com->txBuffer[3 + actualBit/8] & (~(0x01 << actualBit%8)));
						}
						actualBit++;
					}
				
					com->txLength = 3 + com->txBuffer[2]  + 2;				
					sendSlaveResponse(com);
				}
			break;
			#endif
		
			#if MAX_DISCRETE_INPUTS > 0
			case READ_DISCRETE_INPUTS:
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				memorylength = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
				shift = memoryadress % 8;
			
				// 0x0001 <= Quantity of Outputs <= 0x07B0
				if (!((memorylength >= 0x0001) && (memorylength <= 0x07D0))) exceptionCode(com, READ_DISCRETE_INPUTS, ILLEGAL_DATA_VALUE);
				// Starting Address == OK
				else if (memoryadress >= (DISCRETE_INPUT_START_ADRESS + MAX_DISCRETE_INPUTS)) exceptionCode(com, READ_DISCRETE_INPUTS, ILLEGAL_DATA_ADDRESS);
				// Starting Address + Quantity of Outputs == OK
				else if (memoryadress + memorylength > (DISCRETE_INPUT_START_ADRESS + MAX_DISCRETE_INPUTS)) exceptionCode(com, READ_DISCRETE_INPUTS, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = READ_DISCRETE_INPUTS;
					if (memorylength%8) com->txBuffer[2] = (memorylength/8)+1;
					else com->txBuffer[2] = memorylength/8;
				
					actualBit = 0;
					clearTxBuffer(com);
					// Read Discrete Inputs and create response
					for (i=0; i < memorylength; i++)
					{
						if ((data->discreteInputs[(shift + actualBit)/8] >> (shift + actualBit)%8) & 0x01)
						{
							com->txBuffer[3 + actualBit/8] = (0x01 << actualBit%8) | (com->txBuffer[3 + actualBit/8] & (~(0x01 << actualBit%8)));
						}
						else
						{
							com->txBuffer[3 + actualBit/8] = (com->txBuffer[3 + actualBit/8] & (~(0x01 << actualBit%8)));
						}
						actualBit++;
					}
					com->txLength = 3 + com->txBuffer[2]  + 2;
					sendSlaveResponse(com);
				}
			break;
			#endif
		
			#if MAX_HOLDING_REGISTERS > 0
			case READ_HOLD_REGISTERS: //  Read Holding Registers Code 03
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				memorylength = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
			
				// 0x0001 <= Quantity of Registers <= 0x007D
				if ((memorylength < 0x0001) || (memorylength > 0x007D)) exceptionCode(com, READ_HOLD_REGISTERS, ILLEGAL_DATA_VALUE);
				// Starting Address == OK
				else if (memoryadress >= (HOLDING_REGISTERS_START_ADRESS + MAX_HOLDING_REGISTERS)) exceptionCode(com, READ_HOLD_REGISTERS, ILLEGAL_DATA_ADDRESS);
				// Starting Address + Quantity of Registers == OK
				else if ((memoryadress + memorylength) > ((HOLDING_REGISTERS_START_ADRESS + MAX_HOLDING_REGISTERS))) exceptionCode(com, READ_HOLD_REGISTERS, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = READ_HOLD_REGISTERS;
					com->txBuffer[2] = memorylength * 2;
				
					// Read Hold Registers and create response
					for (i=0; i < memorylength ; i++)
					{
						com->txBuffer[(i*2)+3] = (uint8_t)((data->holdingRegisters[i+memoryadress] >> 8) & 0x00FF);
						com->txBuffer[(i*2+1)+3] = (uint8_t)((data->holdingRegisters[i+memoryadress]) & 0x00FF);
					}
					com->txLength = 3 + memorylength*2 + 2;
					sendSlaveResponse(com);
				}
			break;
			#endif
		
			#if MAX_INPUT_REGISTERS > 0
			case READ_INPUT_REGISTERS:
			// Read Input Registers
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				memorylength = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
			
				// 0x0001 <= Quantity of Registers <= 0x007D
				if ((memorylength < 0x0001) || (memorylength > 0x007D)) exceptionCode(com, READ_INPUT_REGISTERS, ILLEGAL_DATA_VALUE);
				// Starting Address == OK
				else if (memoryadress >= (INPUT_REGISTERS_START_ADRESS + MAX_INPUT_REGISTERS)) exceptionCode(com, READ_INPUT_REGISTERS, ILLEGAL_DATA_ADDRESS);
				// Starting Address + Quantity of Registers == OK
				else if ((memoryadress + memorylength) > ((INPUT_REGISTERS_START_ADRESS + MAX_INPUT_REGISTERS ))) exceptionCode(com, READ_INPUT_REGISTERS, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = READ_INPUT_REGISTERS;
					com->txBuffer[2] = memorylength * 2;
				
					// Read Input Registers and create response
					for (i=0; i < memorylength ; i++)
					{
						com->txBuffer[(i*2)+3] = (uint8_t)((data->inputRegisters[i+memoryadress] >> 8) & 0x00FF);
						com->txBuffer[(i*2+1)+3] = (uint8_t)((data->inputRegisters[i+memoryadress]) & 0x00FF);
					}
					com->txLength = 3 + memorylength*2 + 2;
					sendSlaveResponse(com);
				}		
			break;
			#endif
		
			#if MAX_COILS > 0
			case WRITE_SINGLE_COIL: // Code 05 Write Single Coil
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				uint8_t target = memoryadress / 8;
				shift = memoryadress % 8;
				uint16_t value = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
			
				//Output Value == 0x0000 OR 0xFF00
				if ((value != 0xFF00) && (value != 0x000)) exceptionCode(com, WRITE_SINGLE_COIL, ILLEGAL_DATA_VALUE);
				// Output Address == OK
				else if (memoryadress >= (COILS_START_ADRESS + MAX_COILS)) exceptionCode(com, WRITE_SINGLE_COIL, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = WRITE_SINGLE_COIL;
					com->txBuffer[2] = com->rxBuffer[2];
					com->txBuffer[3] = com->rxBuffer[3];
					com->txBuffer[4] = com->rxBuffer[4];
					com->txBuffer[5] = com->rxBuffer[5];
				
					// Write single coil and create response
					if (value == 0x0000)
					{
						data->coils[target] = (data->coils[target] & (~(0x01 << shift)));
					}
					else if (value == 0xFF00)
					{
						data->coils[target] = (0x01 << shift) | (data->coils[target] & (~(0x01 << shift)));
					}
								
					com->txLength = 6 + 2;
					sendSlaveResponse(com);				
				}
			break;
			#endif
		
			#if MAX_HOLDING_REGISTERS > 0
			case WRITE_SINGLE_REGISTER: // Code 06 Write Single Register
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);

				// Starting Address == OK
				if (memoryadress >= HOLDING_REGISTERS_START_ADRESS + MAX_HOLDING_REGISTERS) exceptionCode(com, WRITE_SINGLE_REGISTER, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = WRITE_SINGLE_REGISTER;
					com->txBuffer[2] = com->rxBuffer[2];
					com->txBuffer[3] = com->rxBuffer[3];
					com->txBuffer[4] = com->rxBuffer[4];
					com->txBuffer[5] = com->rxBuffer[5];
				
					// write single register and create response
					data->holdingRegisters[memoryadress] = ((uint16_t)(com->rxBuffer[4] << 8) & 0xFF00) | ((uint16_t)com->rxBuffer[5] & 0x00FF);
				
					com->txLength = 6 + 2;
					sendSlaveResponse(com);					
				}
			break;
			#endif
		
			#if MAX_COILS > 0
			case WRITE_MULTIPLE_COILS: // Code 15 0f Write Multiple Coils
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				memorylength = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
				uint8_t bits = memorylength;
				uint8_t targetstart = memoryadress / 8;
				shift = memoryadress % 8;
				uint16_t bit;
			
				//0x0001 <= Quantity of Outputs <= 0x07B0
				if ((memorylength < 0x0001) || (memorylength > 0x07B0)) exceptionCode(com, WRITE_MULTIPLE_COILS, ILLEGAL_DATA_VALUE);
				// Starting Address == OK
				else if (memoryadress >= (COILS_START_ADRESS + MAX_COILS)) exceptionCode(com, WRITE_MULTIPLE_COILS, ILLEGAL_DATA_ADDRESS);
				// Starting Address + Quantity of Outputs == OK
				else if (memoryadress + memorylength >= (COILS_START_ADRESS + MAX_COILS)) exceptionCode(com, WRITE_MULTIPLE_COILS, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = WRITE_MULTIPLE_COILS;
					com->txBuffer[2] = com->rxBuffer[2];
					com->txBuffer[3] = com->rxBuffer[3];
					com->txBuffer[4] = com->rxBuffer[4];
					com->txBuffer[5] = com->rxBuffer[5];
				
					bit = 0;
					// write multiple coils and create response
					for (i=0; i < bits; i++)
					{
						if (com->rxBuffer[7 + (bit/8)] >> bit%8 & 0x01)
						{
							data->coils[targetstart + (bit+shift)/8] = (0x01 << (bit+shift)%8) | (data->coils[targetstart + (bit+shift)/8] & (~(0x01 << (bit+shift)%8)));
						}
						else
						{
							data->coils[targetstart + (bit+shift)/8] = (data->coils[targetstart + (bit+shift)/8] & (~(0x01 << (bit+shift)%8)));
						}
					bit++;							
					}				
					com->txLength = 6 + 2;
					sendSlaveResponse(com);
				}
			break;
			#endif
		
			#if MAX_HOLDING_REGISTERS > 0
			case WRITE_MULTIPLE_REGISTERS:  //Code 16 - Write Multiple registers
				memoryadress = (com->rxBuffer[2] << 8) | (com->rxBuffer[3] & 0xFF);
				memorylength = (com->rxBuffer[4] << 8) | (com->rxBuffer[5] & 0xFF);
				uint16_t byteCount = com->rxBuffer[6];
			
				// 0x0001 <= Quantity of Registers <= 0x007B
				if ((memorylength < 0x0001) || (memorylength > 0x007B)) exceptionCode(com, WRITE_MULTIPLE_REGISTERS, ILLEGAL_DATA_VALUE);
				// Byte Count == Quantity of Registers x 2
				else if (byteCount != memorylength * 2) exceptionCode(com, WRITE_MULTIPLE_REGISTERS, ILLEGAL_DATA_VALUE);
				// Starting Address == OK
				else if (memoryadress >= (INPUT_REGISTERS_START_ADRESS + MAX_INPUT_REGISTERS)) exceptionCode(com, WRITE_MULTIPLE_REGISTERS, ILLEGAL_DATA_ADDRESS);
				// Starting Address + Quantity of Registers == OK
				else if ((memoryadress + memorylength) > ((INPUT_REGISTERS_START_ADRESS + MAX_INPUT_REGISTERS ))) exceptionCode(com, WRITE_MULTIPLE_REGISTERS, ILLEGAL_DATA_ADDRESS);
				else
				{
					com->txBuffer[0] = SLAVEID;
					com->txBuffer[1] = WRITE_MULTIPLE_REGISTERS;
					com->txBuffer[2] = com->rxBuffer[2]; // Starting Adress Hi
					com->txBuffer[3] = com->rxBuffer[3]; // Starting Adress Lo
					com->txBuffer[4] = com->rxBuffer[4]; // Quantity of Registers Hi 
					com->txBuffer[5] = com->rxBuffer[5]; // Quantity of Registers Lo
				
					// write multiple register and create response
					for (i=0; i < memorylength ; i++)
					{
						data->holdingRegisters[memoryadress+i] = ((uint16_t)(com->rxBuffer[(i*2)+7] << 8) & 0xFF00) |  ((uint16_t)com->rxBuffer[(i*2+1)+7] & 0x00FF);
					}			
					com->txLength = 6 + 2;
					sendSlaveResponse(com);
				}		
			break;
			#endif	
		
			case ERROR:
			// do nothing
			break;
		
			default: // response illegal function code
			exceptionCode(com, com->rxBuffer[1], ILLEGAL_FUNCTION);
			break;	
		}
	}
	
	if (com->rxDone)
	{
		com->rxActive = 0;
		com->rxDone = 0;
		com->rxLength = 0;
	}
}

void exceptionCode(mbComm *com, uint16_t function, uint8_t exception){
	uint16_t crc;
	com->txBuffer[0] = SLAVEID;
	com->txBuffer[1] = function | 0x80; // Function Code MSB 1
	com->txBuffer[2] = exception;
	com->txLength = 5;
	crc = crc16(&com->txBuffer[0], com->txLength - 2);
		
	com->txBuffer[com->txLength-1] = (uint8_t)((crc >> 8) & 0x00FF);
	com->txBuffer[com->txLength-2] = (uint8_t)((crc) & 0x00FF);
	sendSlaveResponse(com);
	return;
}

void clearTxBuffer(mbComm *com){
	uint32_t i;
	for (i=3; i<BUFFERLENGTH;i++){
		com->txBuffer[i] = 0;
	}
	return;
}

void sendSlaveResponse(mbComm *com){
	uint16_t crc;
	
	crc = crc16(&com->txBuffer[0], com->txLength - 2);
	com->txBuffer[com->txLength-1] = (uint8_t)((crc >> 8) & 0x00FF);
	com->txBuffer[com->txLength-2] = (uint8_t)((crc) & 0x00FF);
	
	MBsendMessage(com->txBuffer, com->txLength);
	return;
}

void receiveModbusByte(mbComm *com, uint8_t x){
	
	com->rxBuffer[com->rxLength] = x; // Zeichen in Buffer schreiben
	com->rxLength++;	// Länge um 1 erhöhen
	if(com->rxLength > BUFFERLENGTH)
	{
		com->rxLength = BUFFERLENGTH-1;
	}
	com->rxActive = 1;	// Empfang auf Aktiv setzen
	return;
}

// CRC 16
uint16_t crc16(uint8_t *x, uint8_t lentgh){
	static const uint16_t table[256] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 };

	uint8_t xor = 0;
	uint16_t crc = 0xFFFF;

	while(lentgh--)
	{
		xor = (*x++) ^ crc;
		crc >>= 8;
		crc ^= table[xor];
	}

	return crc;
}

