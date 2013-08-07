/*
=================================================================================
 Name        : I2CSlave.ino
 Version     : 0.1

 Copyright (C) 2012 by Andre Wussow, 2012, desk@binerry.de

 Description :
     Sample of controlling an Arduino connected to Raspberry Pi via I2C.

	 Recommended connection (http://www.raspberrypi.org/archives/384):
	 Arduino pins      I2C-Shifter      Raspberry Pi
	 GND                                P06  - GND
	 5V                5V
	 SDA               SDA2
	 SCL               SCL2
	                   3V3              P01 - 3.3V
	                   SDA1             P03 - SDA
	                   SCL1             P05 - SCL
	 D2                                               LED1 with 1k resistor to GND
	 D3                                               LED2 with 1k resistor to GND
	 D4                                               LED3 with 1k resistor to GND
	 D5                                               Relay with transistor driver

================================================================================
*/

#define DEBUG TRUE

#include "SMBusSlave.h"
#include "Wire.h"

/*
  General Setup
*/

// RTC i2c address
#define DS1307_I2C_ADDRESS 0x68

// define i2c commands
#define LED1_ON_COMMAND		1
#define LED1_OFF_COMMAND	2
#define LED2_ON_COMMAND		3
#define LED2_OFF_COMMAND	4
#define LED3_ON_COMMAND		5
#define LED3_OFF_COMMAND	6
#define RELAY_COMMAND		7
#define QUERY_COMMAND		16

// define slave address. Which will be the same as the address specified in the Master application
#define SLAVE_ADDRESS 0x2A 

// SMBus slave object
SMBusSlave *smBus;

// SMBus timer object
SMBusSlave *timer;

// Forward references
byte decToBcd(byte val);
byte bcdToDec(byte val);
void getDateDs1307(byte *second,
          byte *minute,
          byte *hour,
          byte *dayOfWeek,
          byte *dayOfMonth,
          byte *month,
          byte *year);
          
int led1(int value);
int led2(int value);
int led3(int value);
int relay(int value);
int query(int value);

// pin setup
int led1pin = 10;
int led2pin = 11;
int led3pin = 12;
int relaypin = 5;

void setup()
{
  debugBegin(115200);
  debugPrintln("\n\ni2c Slave (Waiting for commands from Raspberry pi)");

  // initialize the digital pins for leds and relay as an output
  pinMode(led1pin, OUTPUT);
  pinMode(led2pin, OUTPUT);  
  pinMode(led3pin, OUTPUT);  
  pinMode(relaypin, OUTPUT);  
  
  // Instantiate the SMBus slave object
  smBus = new SMBusSlave(SLAVE_ADDRESS);

  // Add commands the slave will look for
  smBus->addCommand(LED1_ON_COMMAND, led1, 1);
  smBus->addCommand(LED1_OFF_COMMAND, led1, 0);
  smBus->addCommand(LED2_ON_COMMAND, led2, 1);
  smBus->addCommand(LED2_OFF_COMMAND, led2, 0);
  smBus->addCommand(LED3_ON_COMMAND, led3, 1);
  smBus->addCommand(LED3_OFF_COMMAND, led3, 0);
  smBus->addCommand(QUERY_COMMAND, query, 0);

  // Start slave processing - using the supplied callbacks
  smBus->startProcessing(receiveData, sendData);
  
//  timer = new SMBusSlave(DS1307_I2C_ADDRESS);
  
  // Start timer processing
//  timer->startProcessing(receiveTime, sendTime);

  led1(1);
  delay(1000);
  led2(1);
  delay(1000);
  led3(1);
  delay(1000);
  led3(0);
  delay(1000);
  led2(0);
  delay(1000);
  led1(0);
}

void loop()
{  
}

// callback for received data
void receiveData(int byteCount) 
{
//  debugPrintln("receiveData");
  smBus->receiveData(byteCount);
}

// callback for sending data
void sendData()
{
//  debugPrintln("sendData");
  smBus->sendData();
}

// callback for received data
void receiveTime(int byteCount) 
{
  debugPrintln("receiveTime");
  timer->receiveData(byteCount);
}

// callback for sending data
void sendTime()
{
  debugPrintln("sendTime");
  timer->initData();
  timer->sendData();
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}

// Gets the date and time from the ds1307
void getDateDs1307(byte *second,
          byte *minute,
          byte *hour,
          byte *dayOfWeek,
          byte *dayOfMonth,
          byte *month,
          byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);

  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}

// SMBus slave command processors
int led1(int value)
{
  digitalWrite(led1pin, value);		// switch led 1 on or off
  return 1;				// Return a success status code
}

int led2(int value)
{
  digitalWrite(led2pin, value);		// switch led 2 on or off
  return 1;				// Return a success status code
}

int led3(int value)
{
  digitalWrite(led3pin, value);		// switch led 3 on or off
  return 1;				// Return a success status code
}

int relay(int value)
{
  digitalWrite(relaypin, value);	// switch relay on or off
  return 1;				// Return a success status code
}

int query(int value)
{
  sendTime();
	
  if (1 != 0) return 42;
  
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  debugPrintDEC(hour);
  debugPrint(":");
  debugPrintDEC(minute);
  debugPrint(":");
  debugPrintDEC(second);
  debugPrint("  ");
  debugPrintDEC(month);
  debugPrint("/");
  debugPrintDEC(dayOfMonth);
  debugPrint("/");
  debugPrintDEC(year);
  debugPrint("  Day_of_week:");
  debugPrintDECln(dayOfWeek);
  
  return (int)hour;
}
