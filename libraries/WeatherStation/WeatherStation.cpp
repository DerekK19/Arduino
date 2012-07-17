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


#include "WeatherStation.h"

//Global Variables
unsigned char buffer[BUFFER_LENGTH];

//Function Declarations
int checkPreamble();
int decode();
void parseData();
int alignData();

// The digital IO pin number of the receiver data
static uint8_t RX_PIN = 8;

// The data ready pin
static uint8_t ACK_PIN = 6;

// The data sync pin
static uint8_t SYNC_PIN = 7;

// The state machine;s current state
int state = IDLE;			//Startup state is idle

// Set the pin number for input receiver data
void ws_set_rx_pin(uint8_t pin)
{
    RX_PIN = pin;
}

// Set the pin for the Data ready signal
void ws_set_ack_pin(uint8_t pin)
{
    ACK_PIN = pin;
}

// Set the pin for the Data sync signal
void ws_set_sync_pin(uint8_t pin)
{
    SYNC_PIN = pin;
}

int read_pin()
{
    return digitalRead(RX_PIN);
}

#define PIN8_VALUE ((*portInputRegister(2) & 1) == 1 ? HIGH : LOW)
#define PIN8_HIGH ((*portInputRegister(2) & 1) == 1)
#define PIN8_LOW ((*portInputRegister(2) & 1) == 0)

/**
********************************************************************
\brief Align the incoming packets with correct position in buffer
**********************************************************************/
int alignData() {

		Serial.print("alignData: ");
		int x =buffer[0];
		Serial.println(x);

	if(buffer[0] == 208)
	{
		//Alignment issue, need to delay some bits to get alignment
		while(PIN8_HIGH);	//Wait till pin goes low
		while(PIN8_LOW); //Wait till pin goes high
		while(PIN8_HIGH);	//Wait till pin goes low		
	}
	else if(buffer[0] == 232)
	{
		while(PIN8_HIGH);	//Wait till pin goes low
		while(PIN8_LOW); //Wait till pin goes high
		while(PIN8_HIGH);	//Wait till pin goes low

		while(PIN8_LOW); //Wait till pin goes high
		while(PIN8_HIGH);	//Wait till pin goes low
	}
	else if(buffer[0] == 244)
	{
		while(PIN8_HIGH);	//Wait till pin goes low
		while(PIN8_LOW); //Wait till pin goes high
		while(PIN8_HIGH);	//Wait till pin goes low

		while(PIN8_LOW); //Wait till pin goes high
		while(PIN8_HIGH);	//Wait till pin goes low

		while(PIN8_LOW); //Wait till pin goes high
		while(PIN8_HIGH);	//Wait till pin goes low
	}
	else if(buffer[0] == 161)
	{
		//Don't do anything, all aligned.
	}
	else
	{
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
//	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Humidity = %d\r\n",(int)buffer[3]);
	Serial.println(stringBuffer);

	//Temperature requires a little more care as it is a signed value
	temperature = (signed char)buffer[2];
	sprintf(stringBuffer,"Temperature = %0.1f\r\n",(temperature * 0.1) + 11.2);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Wind = %0.1fkmh\r\n",buffer[4] * 1.2222222);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Gust = %0.1fkmh\r\n",buffer[5] * 1.2222222);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Direction = %0.1f Degrees\r\n",(buffer[8] & 0x0F) * 22.5);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 0 = %d\r\n",(int)buffer[0]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 1 = %d\r\n",(int)buffer[1]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 2 = %d\r\n",(int)buffer[2]);
	Serial.println(stringBuffer);
	
	sprintf(stringBuffer,"Byte 3 = %d\r\n",(int)buffer[3]);
	Serial.println(stringBuffer);	

	sprintf(stringBuffer,"Byte 4 = %d\r\n",(int)buffer[4]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 5 =  %d\r\n",(int)buffer[5]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 6 =  %d\r\n",(int)buffer[6]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 7 =  %d\r\n",(int)buffer[7]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 8 =  %d\r\n",(int)buffer[8]);
	Serial.println(stringBuffer);
	
	sprintf(stringBuffer,"Byte 9 =  %d\r\n",(int)buffer[9]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 10 =  %d\r\n",(int)buffer[10]);
	Serial.println(stringBuffer);

	sprintf(stringBuffer,"Byte 11 =  %d\r\n\r\n",(int)buffer[11]);
	Serial.println(stringBuffer);
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
	int result;

Serial.println("decode");

	for(i=0;i < BITS_TO_DECODE;i++) {
		buffer[bufferPointer] <<= 1;	//Shift to next bit

		while(PIN8_LOW);	//Pin is in low state, wait here for a trigger
		delayMicroseconds(250);
		if(PIN8_LOW) {
			//Failed. Transmit how many bits we got through then return.

			Serial.print("Length = ");
			Serial.print(i);
			Serial.print(" ");

			return -1;
		}

		delayMicroseconds(750);
		temp = PIN8_VALUE;		//The state of the pin measured here determines the logic level. 1 = logic 0

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

		while(PIN8_HIGH);	//Wait here till the pin goes low again.

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
	unsigned long lapsedTime, startTime;

//	Serial.println("Preamble?");

	digitalWrite(ACK_PIN, 1);

	// receive pin should already be in a low state at this stage
	for(i=0;i<6;i++)
	{
		while(PIN8_LOW);		//Pin is in low state, wait here for it to go high
		startTime = micros();	//Trigger occured, start the timer
		while(PIN8_HIGH);		//Wait till the data line goes low
		lapsedTime = micros()-startTime;

//	Serial.print(THRESH05_LOW);
//	Serial.print(" | ");
//	Serial.print(lapsedTime);
//	Serial.print(" | ");
//	Serial.print(THRESH05_HIGH);
//	Serial.println("");

		if(lapsedTime > THRESH05_LOW && lapsedTime < THRESH05_HIGH)
		{
			preambleCount++;	//Found another preamble so count up
		}
		else break;
	}

	Serial.println(preambleCount);

	//Checking of preamble done. If count is higher than 6 then it is ok.
	if(preambleCount >= 5)
	{
		digitalWrite(SYNC_PIN, 1);
		return PREAMBLE_FOUND;
	}
	else
	{
		return IDLE;
	}
}


void clearBuffer()
 {
	int i;

	//Clear the buffer first. Buffer needs to be reset to all zeros before start.
	for(i=0;i<BUFFER_LENGTH;i++)
	{
		buffer[i] = 0;
	}
}

/**
********************************************************************
\brief Main State Machine for decoding wireless
**********************************************************************/
void startWirelessWeather() {
	unsigned long lapsedTime, startTime;
	int i;
	int result;

	clearBuffer();
	
	pinMode(RX_PIN, INPUT);
	pinMode(ACK_PIN, OUTPUT);
	pinMode(SYNC_PIN, OUTPUT);

	state=IDLE;
}

void getWirelessWeather()
{
	unsigned long atLow, toHigh, toLow, lapsedTime1, lapsedTime2, startTime;
	int i;
	int result;
	int preambleCount = 0;
    digitalWrite(ACK_PIN, 0);
    digitalWrite(SYNC_PIN, 0);
	switch(state)
	{
		case IDLE:
//			Serial.println("IDLE");
			//Wait for pin to go high to low. When found see how long the pulse is.
			//If pulse is 0.5ms long then see if there are another 7 of them.
			while(PIN8_HIGH);	//Wait till pin has actually gone low

			toLow = micros();
			for(i=0;i<6;i++)
			{
				atLow = toLow;
				while(PIN8_LOW);		//Pin is in low state, wait here for it to go high
				toHigh = micros();		//Trigger occured, start the timer
				while(PIN8_HIGH);		//Wait till the data line goes low
				toLow = micros();
				lapsedTime1 = toHigh-atLow;
				lapsedTime2 = toLow-toHigh;

				if(lapsedTime2 > THRESH05_LOW && lapsedTime2 < THRESH05_HIGH)
				{
//				    digitalWrite(ACK_PIN, 1);
					preambleCount++;	//Found another preamble so count up
				}
				else
				{
					if (abs(lapsedTime1 - lapsedTime2) < 100)
					{
/*
	Serial.print(preambleCount);
	Serial.print(" ");
	Serial.print(THRESH05_LOW);
	Serial.print(" | ");
	Serial.print(lapsedTime1);
	Serial.print(" | ");
	Serial.print(lapsedTime2);
	Serial.print(" | ");
	Serial.print(THRESH05_HIGH);
	Serial.println("");
*/
					}
				 	break;
				}
			}
			//Checking of preamble done. If count is higher than 6 then it is ok.
			if(preambleCount >= 6)
			{
				digitalWrite(SYNC_PIN, 1);
				state = PREAMBLE_FOUND;
			}
			else
			{
				state = IDLE;
			}
			break;
		
		case PREAMBLE_FOUND:
			//At this stage we have confirmed that the correct preamble has been received. Now we go about
			//Decoding the rest of the data. The receive pin should currently be in a low state. There are about 80 bits to decode.
			Serial.println("PREAMBLE");
					
			result = decode();
			if(result ==0) {
				state = PARSE;
			}else{
				Serial.println("Error");
				state = IDLE;
				clearBuffer();
			}
			break;

		case PARSE:
			Serial.println("PARSE");
			parseData();
			state = IDLE;
			clearBuffer();
			break;

		case PACKET_RECEIVED:
			break;
	}
}
