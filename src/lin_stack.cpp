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
 *  LIN STACK for MCP2025 
 *  v2.0
 *
 *  Short description: 
 *  Comunication stack for Arduino Nano and MC2025 LIN transceiver. 
 *  Can be modified for any Arduino board with UART available and any LIN slave.
 *  
 *  Author: Blaž Pongrac B.S., RoboSap, Institute of Technology, Ptuj (www.robosap-institut.eu)
 *  Author: Blake Vermeer
 *  
 *  Arduino IDE 1.8.2
 *  RoboSap, Institute of Technology, September 2016
 */ 

#include <lin_stack.h>

/* LIN PACKET:
   It consist of:
    ___________ __________ _______ ____________ _________ 
   |           |          |       |            |         |
   |Synch Break|Synch Byte|ID byte| Data Bytes |Checksum |
   |___________|__________|_______|____________|_________|
   
   Every byte have start bit and stop bit and it is send LSB first.
   Synch Break - 13 bits of dominant state ("0"), followed by 1 bit recesive state ("1")
   Synch Byte - Byte for Bound rate syncronization, always 0x55
   ID Byte - consist of parity, length and address; parity is determined by LIN standard and depends from address and message length
   Data Bytes - user defined; depend on devices on LIN bus
   Checksum - inverted 256 checksum; data bytes are sumed up and then inverted
*/

// CONSTRUCTORS
lin_stack::lin_stack(boolean sleepEn){
	sleepEnable = sleepEn;
	sleep_config(); // Configure Sleep pin for transceiver
}

lin_stack::lin_stack(boolean sleepEn, byte ident){
	sleepEnable = sleepEn;
	sleep_config(); // Configuration of Sleep pin for transceiver
	identByte = ident; // saving ident to private variable
	sleep(1); // Transceiver is always in Normal Mode
}

// PUBLIC METHODS
// WRITE methods
// Creates a LIN packet and then send it via USART(Serial) interface.
int lin_stack::write(byte ident, byte data[], byte data_size){

	// Calculate checksum
	byte suma = 0;
	for(int i=0;i<data_size;i++) suma = suma + data[i];
	suma = suma + 1;
	byte checksum = 255 - suma;

	// Start interface
	sleep(1); // Go to Normal mode

	// Synch Break
	serial_pause(13);

	// Send data via Serial interface
	Serial.begin(baud_rate); // config Serial
	Serial.write(0x55); // write Synch Byte to serial
	Serial.write(ident); // write Identification Byte to serial

	for(int i=0;i<data_size;i++)
	{
		Serial.write(data[i]); // write data to serial
	} 

	Serial.write(checksum); // write Checksum Byte to serial
	Serial.end(); // clear Serial config

	sleep(0); // Go to Sleep mode

	return 1;
}

int lin_stack::writeRequest(byte ident){

	// Create Header
	byte header[2]= {0x55, ident};

	// Start interface
	sleep(1); // Go to Normal mode

	// Synch Break
	serial_pause(13);

	// Send data via Serial interface
	Serial.begin(baud_rate); // config Serial
	Serial.write(header,2); // write data to serial
	Serial.end(); // clear Serial config

	sleep(0); // Go to Sleep mode

	return 1;
}

int lin_stack::writeResponse(byte data[], unsigned int data_size){

	// Calculate checksum
	byte suma = 0;

	for(unsigned int i=0;i<data_size;i++) suma = suma + data[i];
	suma = suma + 1;
	byte checksum = 255 - suma;

	// Start interface
	sleep(1); // Go to Normal mode

	// Send data via Serial interface
	Serial.begin(baud_rate); // config Serial
	Serial.write(data, data_size); // write data to serial
	Serial.write(checksum); // write data to serial
	Serial.end(); // clear Serial config

	sleep(0); // Go to Sleep mode

	return 1;
}

int lin_stack::writeStream(byte data[], unsigned int data_size){

	// Start interface
	sleep(1); // Go to Normal mode

	// Synch Break
	serial_pause(13);

	// Send data via Serial interface
	Serial.begin(baud_rate); // config Serial

	for(unsigned int i=0;i<data_size;i++)
	{
		Serial.write(data[i]);
	} 

	Serial.end(); // clear Serial config

	sleep(0); // Go to Sleep mode

	return 1;
}

// READ methods
// Read LIN traffic and then proces it.
void lin_stack::setSerial(){ // Only needed when receiving signals
	Serial.begin(10417); // Configure Serial
}

int lin_stack::read(byte data[], unsigned int data_size){

	byte rec[data_size+3];

	if(Serial.read() != -1){ // Check if there is an event on LIN bus

		Serial.readBytes((char*)rec,data_size+3);
		
		if((validateParity(rec[1]))&(validateChecksum(rec,data_size+3))){

			for(unsigned int j=0;j<data_size;j++)
			{
				data[j] = rec[j+2];
			}

			return 1;

		} else
		{
			return -1;
		}	
	}

	return 0;
}

int lin_stack::readStream(byte data[], unsigned int data_size){

	byte rec[data_size];

	if(Serial.read() != -1){ // Check if there is an event on LIN bus

		Serial.readBytes((char*)rec,data_size);

		for(unsigned int j=0;j<data_size;j++)
		{
			data[j] = rec[j];
		}

		return 1;
	}

	return 0;
}


// PRIVATE METHODS
int lin_stack::serial_pause(int no_bits){

	// Calculate delay needed for 13 bits, depends on baud rate
	unsigned int del = period*no_bits; // delay for number of bits (no-bits) in microseconds, depends on period

	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);
	delayMicroseconds(del); // delay
	digitalWrite(1, HIGH);
	pinMode(1, INPUT);

	return 1;
}

int lin_stack::sleep(byte sleep_state){
	if(sleepEnable == true)
	{
		if(sleep_state==1){ // Go to Normal mode
			digitalWrite(2, HIGH);
		}else if(sleep_state==0){ // Go to Sleep mode
			digitalWrite(2, LOW);
		}
		delayMicroseconds(20); // According to TJA1021 datasheet this is needed for propper working
	}

	return 1;
}

int lin_stack::sleep_config(){

	// Use Pin D2 as the CS/LWAKE pin to the MCP2025 transciever

	// Set TX = 0 and CS = 1 to transition the LIN transciever to the TX OFF state
	// Pin 1 is Serial TX on the Arduino Nano and CS is connected to pin 2.
	pinMode(1, OUTPUT);
	digitalWrite(1, LOW);
	delayMicroseconds(200);
	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);
	delayMicroseconds(200);

	return 1;
}

boolean lin_stack::validateParity(byte ident) {
	if(ident == identByte)
		return true;
	else
		return false;
}

boolean lin_stack::validateChecksum(byte data[], unsigned int data_size){
	byte checksum = data[data_size-1];
	byte suma = 0;
	for(unsigned int i=2;i<data_size-1;i++) suma = suma + data[i];
	byte v_checksum = 255 - suma - 1;
	if(checksum==v_checksum)
		return true;
	else
		return false;
} 
