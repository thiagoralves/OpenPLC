//-----------------------------------------------------------------------------
// Copyright 2015 Thiago Alves
//
// Based on the LDmicro software by Jonathan Westhues
// This file is part of the OpenPLC Software Stack.
//
// OpenPLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenPLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenPLC.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// This file is to be used in conjuction with OPLC_Compiler generated code.
// It is the "glue" that holds all parts of the OpenPLC together.
// Thiago Alves, Oct 2015
//-----------------------------------------------------------------------------

#include <pthread.h>

#ifndef LADDER_H
#define LADDER_H

#define BOOL bool
#define SWORD int

#define EXTERN_EVERYTHING
#define NO_PROTOTYPES

//Internal buffers for I/O and memory. These buffers are defined in the
//auto-generated code by the OPLC_Compiler
#define BUFFER_SIZE		10000

extern SWORD AnalogInputBuffer0[BUFFER_SIZE]; //Input Registers
extern SWORD AnalogOutputBuffer0[BUFFER_SIZE]; //Holding Registers
extern BOOL DiscreteInputBuffer0[BUFFER_SIZE];
extern BOOL CoilsBuffer0[BUFFER_SIZE];

extern SWORD MemBuffer0[BUFFER_SIZE];
extern SWORD MemBuffer1[BUFFER_SIZE];
extern SWORD MemBuffer2[BUFFER_SIZE];

//lock for the buffer
extern pthread_mutex_t bufferLock;

//----------------------------------------------------------------------
//FUNCTION PROTOTYPES
//----------------------------------------------------------------------

//hardware_layer.cpp
void initializeHardware();
void sendOutput(unsigned char *sendBytes, unsigned char *recvBytes);
void updateBuffers();

//openplc.cpp
void *modbusThread();
void PlcCycle(void); //read inputs, process them according to the ladder software and set outputs

//server.cpp
int createSocket(int port);
int waitForClient(int socket_fd);
int listenToClient(int client_fd, unsigned char *buffer);
void processMessage(unsigned char *buffer, int bufferSize);
void *handleConnections(void *arguments);
void startServer(int port);

//modbus.cpp
//Declaration of the supported modbus functions and the respective function code
enum MB_FC
{
	MB_FC_NONE						= 0,
	MB_FC_READ_COILS				= 1,
	MB_FC_READ_INPUTS				= 2,
	MB_FC_READ_HOLDING_REGISTERS	= 3,
	MB_FC_READ_INPUT_REGISTERS		= 4,
	MB_FC_WRITE_COIL				= 5,
	MB_FC_WRITE_REGISTER			= 6,
	MB_FC_WRITE_MULTIPLE_COILS		= 15,
	MB_FC_WRITE_MULTIPLE_REGISTERS	= 16,
	MB_FC_ERROR						= 255
};

enum MB_ERROR
{
	ERR_NONE						= 0,
	ERR_ILLEGAL_FUNCTION			= 1,
	ERR_ILLEGAL_DATA_ADDRESS		= 2,
	ERR_ILLEGAL_DATA_VALUE			= 3,
	ERR_SLAVE_DEVICE_FAILURE		= 4,
	ERR_SLAVE_DEVICE_BUSY			= 6
};

//Definition of the Modbus class
class Modbus
{
	public:
		Modbus(unsigned char *request, int size); //constructor
		int Run(unsigned char *reply); //perform the modbus operations

		//Controls for debug
		int Runs, Reads, Writes;

	private:
		unsigned char ByteArray[260]; //message received / sent
		int MessageLength; //size of the reply message
		MB_FC FC; //Function code received / sent
		MB_ERROR ER; //Modbus error
		void SetFC(int fc); //method to identify the function code received
		int word(unsigned char byte1, unsigned char byte2); //method to create a word from two bytes
		void ReadCoils();
		void ReadDiscreteInputs();
		void ReadHoldingRegisters();
		void ReadInputRegisters();
		void WriteCoil();
		void WriteRegister();
		void WriteMultipleCoils();
		void WriteMultipleRegisters();
		void ModbusError();
};

#endif
