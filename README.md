# uModbus Slave
This is a small Modbus Slave RTU Stack

Include the 4 files modbusConf.h, modbusSlave.h, modbusSlave.c and modbusio.c to your project.

The following function codes have been implemented:

* 0x01 Read Coils
* 0x02 Read Discrete Inputs
* 0x03 Read Holding Registers
* 0x04 Read Input Registers
* 0x05 Write Single Coil
* 0x06 Write Single Registers
* 0x0F Write Multiple Coils
* 0x10 Write Multiple Holding Registers

## Configuration
The configuration is made in modbusConf.h.

* BUFFERLENGTH - Size of the send and receive buffer. Modbus specification Max. 256.
* MAX_DISCRETE_INPUTS - Maximum number of discrete inputs
* MAX_COILS - Maximum number of coils
* MAX_INPUT_REGISTERS - Maximum number of input registers
* MAX_HOLDING_REGISTERS - Maximum number of holding registers

If the number of data is 0, the associated functions are not compiled and an "Illegal Function Code" is returned when a request is made.

## Integration
Include the modbusSlave.h to your project. You must also create the following variables.

* mbDataMapping modbusData - this contains your data. Inputs / Coils / Input Registers and Holding Registers
* mbComm modbusComm - this is used for communication. Send/Receive Buffer etc.

In the program, only modbusSlaveCyclic(&modbusComm ,&modbusData) needs to be called cyclically.

## Adaptation to the hardware
The adaption to the hadrware must be done in modbusio.c.

* For every received byte call receiveModbusByte(&modbusComm, -received byte-) with the correspondig byte
* In MBsendMessage() a function must be called to send a the buffer with a given length
* MBcheckTimer() must return a 1 when a whole frame ist received. Modbus spec says after 3,5 Chars without a new char received the transmission ist completed
* MBTimer() is used to wait this 3,5 chars. It's called in a 1ms Systemtimer. You can use your own function for that. It's only important to return the 1 in MBcheckTimer.
