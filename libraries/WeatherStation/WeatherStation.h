/**
* /File: WeatherStation.h
* /Author: Leon Burkamshaw (Burkotronics)
* /Date: 10 April 2012
* /Description: State Machine to decode Jaycar weather stations
*/
#define IDLE	0
#define PREAMBLE_FOUND	1
#define PARSE	2
#define PACKET_RECEIVED	3
#define RX_PIN PORTBbits.RB6

#define THRESH05_LOW 1981
#define THRESH05_HIGH 2972
#define BUFFER_LENGTH 12
#define BITS_TO_DECODE 80
#define DEVICE_ID 161
#define CRC_POLY      0xAB
//Functions
void startWirelessWeather();	//Starts state machine

