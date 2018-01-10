/*  Copyright (c) 2016 Macchina
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *  
 *  LIN STACK for TJA1021
 *  v2.0
 *
 *  Short description: 
 *  Comunication stack for LIN and TJA1021 LIN transceiver. 
 *  Can be modified for any Arduino board with UART available and any LIN slave.
 *  
 *  Author: Blaž Pongrac B.S., RoboSap, Institute of Technology, Ptuj (www.robosap-institut.eu)
 *  
 *  Arduino IDE 1.6.9
 *  RoboSap, Institute of Technology, September 2016
*/ 

#include <Arduino.h>

/*
	Please, read Getting Started Guide firts.
*/

class lin_stack
{
	public:
	// Constructors
	lin_stack(boolean sleepEn); // Constructor for Master Node
	lin_stack(boolean sleepEn, byte ident); // Constructor for Slave Node
	
	// Methods
	
	// Writing data to bus
	int write(byte add, byte data[], byte data_size); // write whole package
	int writeRequest(byte add); // Write header only
	int writeResponse(byte data[], unsigned int data_size); // Write response only
	int writeStream(byte data[], unsigned int data_size); // Writing user data to LIN bus
	int read(byte data[], unsigned int data_size); // read data from LIN bus, checksum and ident validation
	int readStream(byte data[], unsigned int data_size); // read data from LIN bus
	void setSerial(); // set up Seril communication for receiving data.
	
	// Private methods and variables
	private:
	const unsigned long baud_rate = 10417; // 10417 is best for LIN Interface, most device should work
	const unsigned int period = 96; // in microseconds, 1s/10417
	byte ch = 0; // which channel should be used
	byte identByte; // user defined Identification Byte
	int sleep(byte sleep_state); // method for controlling transceiver modes (0 - sleep, 1 - normal)
	int sleep_config(); // configuration of sleep pins
	int serial_pause(int no_bits); // for generating Synch Break
	boolean validateParity(byte ident); // for validating Identification Byte, can be modified for validating parity
	boolean validateChecksum(byte data[], unsigned int data_size); // for validating Checksum Byte

	boolean sleepEnable;
};