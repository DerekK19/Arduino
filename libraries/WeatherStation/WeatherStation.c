/**
* /File: WeatherStation.c
* /Author: Leon Burkamshaw (Burkotronics)
* /Date: 10 April 2012
Released under the Creative Commons Attribution Share-Alike 3.0 Liscense
http://creativecommons.org/liscenses/by-sa/3.0
Designed by Leon Burkamshaw (Burkotronics)

* Requires a pin to be defined for the wireless receiver
* /Description: State Machine to decode Jaycar weather stations

Protocol:
Byte 0 - Device ID
Byte 1 - Device ID 2   - I assume these numbers are generated randomly on device reset
Byte 2 - Temperature y = 0.1X + 11.2 (Not sure how this will work for numbers below 11.2)
Byte 3 - Humidity (Value  is simply the RH)
Byte 4 - Average wind speed (This value * 1.22 gives km/h. Is a little off at the moment)
Byte 5 - Wind Gust (Same as previous)
Byte 6 - Assuming this is rain in 0.3mm increments? Base station adds this up
Byte 7 - Unknown - Accumalated rain?
Byte 8 - Lower Nibble is wind direction. Direction is 22.5 degrees times this value
Byte 9 - CRC
*/

//TODO: Instead of terminating if less then 80 bits, just terminate if low bit count. 
//TODO: Take care of the case of learning a new device ID code


#include <p33fj128gp804.h>

#include "WeatherStation.h"
#include <main.h>
#include <uart.h>
#include <system.h>
#include <string.h>
#include <stdio.h>

//Global Variables
unsigned char buffer[BUFFER_LENGTH];

//Function Declarations
int checkPreamble();
int decode();
void parseData();
int alignData();

#define RX_PIN 8

/**
********************************************************************
\brief Align the incoming packets with correct position in buffer


**********************************************************************/
int alignData() {
	if(buffer[0] == 208) {
		//Alignment issue, need to delay some bits to get alignment
		while(RX_PIN);	//Wait till pin goes low
		while(!RX_PIN); //Wait till pin goes high
		while(RX_PIN);	//Wait till pin goes low

		
	}else if(buffer[0] == 232) {
		while(RX_PIN);	//Wait till pin goes low
		while(!RX_PIN); //Wait till pin goes high
		while(RX_PIN);	//Wait till pin goes low

		while(!RX_PIN); //Wait till pin goes high
		while(RX_PIN);	//Wait till pin goes low
	}else if(buffer[0] == 244) {
		while(RX_PIN);	//Wait till pin goes low
		while(!RX_PIN); //Wait till pin goes high
		while(RX_PIN);	//Wait till pin goes low

		while(!RX_PIN); //Wait till pin goes high
		while(RX_PIN);	//Wait till pin goes low

		while(!RX_PIN); //Wait till pin goes high
		while(RX_PIN);	//Wait till pin goes low

	}else if(buffer[0] == 161) {

		//Don't do anything, all aligned.

	}else {
		return -1;
	}
	return 0;

}

/**
********************************************************************
\brief Parse the raw data to engineering units


**********************************************************************/
void parseData() {
	char stringBuffer[40];
	signed char temperature;

//	sprintf(stringBuffer,"%0.1f,%0.1f\r\n",buffer[4] * 1.2222222,buffer[5] * 1.2222222);
//	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Humidity = %d\r\n",(int)buffer[3]);
	uart1_puts(stringBuffer);

	//Temperature requires a little more care as it is a signed value
	temperature = (signed char)buffer[2];
	sprintf(stringBuffer,"Temperature = %0.1f\r\n",(temperature * 0.1) + 11.2);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Wind = %0.1fkmh\r\n",buffer[4] * 1.2222222);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Gust = %0.1fkmh\r\n",buffer[5] * 1.2222222);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Direction = %0.1f Degrees\r\n",(buffer[8] & 0x0F) * 22.5);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 0 = %d\r\n",(int)buffer[0]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 1 = %d\r\n",(int)buffer[1]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 2 = %d\r\n",(int)buffer[2]);
	uart1_puts(stringBuffer);
	
	sprintf(stringBuffer,"Byte 3 = %d\r\n",(int)buffer[3]);
	uart1_puts(stringBuffer);	

	sprintf(stringBuffer,"Byte 4 = %d\r\n",(int)buffer[4]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 5 =  %d\r\n",(int)buffer[5]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 6 =  %d\r\n",(int)buffer[6]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 7 =  %d\r\n",(int)buffer[7]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 8 =  %d\r\n",(int)buffer[8]);
	uart1_puts(stringBuffer);

	sprintf(stringBuffer,"Byte 9 =  %d\r\n",(int)buffer[9]);
	uart1_puts(stringBuffer);


	sprintf(stringBuffer,"Byte 10 =  %d\r\n",(int)buffer[10]);
	uart1_puts(stringBuffer);


	sprintf(stringBuffer,"Byte 11 =  %d\r\n\r\n",(int)buffer[11]);
	uart1_puts(stringBuffer);


	
}

/**
********************************************************************
\brief Decode the data stream


**********************************************************************/
int decode() {
	int i;
	unsigned char temp;
	int bufferPointer = 0;
	int bitCount = 0;
	char *stringBuffer;
	int result;


	for(i=0;i < BITS_TO_DECODE;i++) {
		buffer[bufferPointer] <<= 1;	//Shift to next bit

		while(!RX_PIN);	//Pin is in low state, wait here for a trigger
		microcontroller_delay_us(250);
		if(RX_PIN == 0) {
			//Failed. Transmit how many bits we got through then return.
//			sprintf(stringBuffer,"Length = %d\r\n",i);
//			uart1_puts(stringBuffer);
//			return -1;
		}

		microcontroller_delay_us(750);
		temp = RX_PIN;		//The state of the pin measured here determines the logic level. 1 = logic 0

		if(temp > 0) {
			//Do nothing as the zero is already there. Just shift byte
		}else{
			buffer[bufferPointer] |= 0x01;
		}

		
		bitCount++;

		if(bitCount > 7 && bufferPointer == 0) {
			result = alignData();	//Check the first byte is the correct value
			if(result <0) {
				return -1;
			}
			bitCount=0;
			bufferPointer++;
		}else if (bitCount > 7){
			bitCount=0;
			bufferPointer++;
		}

		while(RX_PIN);	//Wait here till the pin goes low again.

	}

	return 0;
}




/**
********************************************************************
\brief Check for 7 conseutive preamble pulses


**********************************************************************/
int checkPreamble() {
	int i;
	int preambleCount = 0;
	unsigned long lapsedTime;

	//RX_PIN should already be in a low state at this stage
	for(i=0;i<6;i++) {
		while(!RX_PIN);	//Pin is in low state, wait here for a trigger
					
		swStartStop(1);	//Trigger occured, start the timer
		while(RX_PIN);	//Wait till the data line goes low
		lapsedTime = swStartStop(0);

		if(lapsedTime > THRESH05_LOW && lapsedTime < THRESH05_HIGH) {
			preambleCount++;	//Found another preamble so count up
		}
	
		

	}

	//Checking of preamble done. If count is higher than 6 then it is ok.
	if(preambleCount >= 5) {
		
		return PREAMBLE_FOUND;
	}else{
		return IDLE;
	}


}


void clearBuffer() {
	int i;

	//Clear the buffer first. Buffer needs to be reset to all zeros before start.
	for(i=0;i<BUFFER_LENGTH;i++) {
		buffer[i] = 0;

	}


}


/**
********************************************************************
\brief Main State Machine for decoding wireless


**********************************************************************/
void startWirelessWeather() {
	int state = IDLE;			//Startup state is idle
	unsigned long lapsedTime;
	int i;
	int result;

	clearBuffer();




	while(1) {
		switch(state) {

			case IDLE:
					//Wait for pin to go high to low. When found see how long the pulse is.
					//If pulse is 0.5ms long then see if there are another 7 of them.
					while(RX_PIN);	//Wait till pin has actually gone low
					while(!RX_PIN);	//Pin is in low state, wait here for a trigger
					
					swStartStop(1);	//Trigger occured, start the timer
					while(RX_PIN);	//Wait till the data line goes low
					lapsedTime = swStartStop(0);

					//If this pulse is 0.5ms, then check there are another 7 of them. If not, then reset.
					if(lapsedTime > THRESH05_LOW && lapsedTime < THRESH05_HIGH) {
						state = checkPreamble();
					}

					break;

			case PREAMBLE_FOUND:
					//At this stage we have confirmed that the correct preamble has been received. Now we go about
					//Decoding the rest of the data. The RX_PIN should currently be in a low state. There are about 80 bits to decode.
					
					result = decode();
					if(result ==0) {
						state = PARSE;
					}else{
						uart1_puts("Error\r\n");
						state = IDLE;
						clearBuffer();
					}


					break;


			case PARSE:
					
					parseData();
					state = IDLE;
					clearBuffer();
					break;



			case PACKET_RECEIVED:

					break;

		}


	}


}
