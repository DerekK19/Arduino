/**
* /File: WeatherStation.h
* /Author: Leon Burkamshaw (Burkotronics)
* /Date: 10 April 2012
* /Description: State Machine to decode Jaycar weather stations
*/

#ifndef WeatherStation_h
#define WeatherStation_h

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#endif

#define IDLE	0
#define PREAMBLE_FOUND	1
#define PARSE	2
#define PACKET_RECEIVED	3
//#define RX_PIN PORTBbits.RB6

#define THRESH05_LOW 1981
#define THRESH05_HIGH 2972
#define BUFFER_LENGTH 12
#define BITS_TO_DECODE 80
#define DEVICE_ID 161
#define CRC_POLY      0xAB

//Functions
void ws_set_rx_pin(uint8_t pin);
void startWirelessWeather();	//Starts state machine
void getWirelessWeather();

#endif