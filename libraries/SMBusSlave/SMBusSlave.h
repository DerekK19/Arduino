/*
=================================================================================
 Name        : SMBusSlave.h
 Version     : 0.1

================================================================================
*/
#include "Arduino.h"

// ensure this library description is only included once:
#ifndef SMBusSlave_h
#define SMBusSlave_h

#if DEBUG == TRUE
#define debugBegin(x)		Serial.begin(x)
#define debugWrite(x)		Serial.write(x)
#define debugPrint(x)		Serial.print(x)
#define debugPrintln		Serial.println
#define debugPrintDEC(x)	Serial.print(x, DEC)
#define debugPrintDECln(x)	Serial.println(x, DEC)
#define debugPrintHEX(x)	Serial.print(x, HEX)
#define debugPrintHEXln(x)	Serial.println(x, HEX)
#else
#define debugBegin(x)		Serial.begin(x)
#define debugWrite(x)
#define debugPrint(x)
#define debugPrintln
#define debugPrintDEC(x)
#define debugPrintDECln(x)
#define debugPrintHEX(x)
#define debugPrintHEXln(x)
#endif

// constants:
#define MAX_COMMANDS 8		// max supported commands

// structs:
// SMBus-Struct:
typedef struct {
  int command;
  int (*callback)(int);		// callback function 
  int arg;					// argument to pass to callback function
  uint8_t isActive   :1;	// true if this command is enabled 
} SMBusCommand_t;

// Commander library interface description:
class SMBusSlave {
  public:
    // constructor:
    SMBusSlave(int address);
    
    // methods:
    void startProcessing(void (*user_onReceive)(int), void (*user_onRequest)());
    void addCommand(int command, int (*callback)(int), int arg);
    int processCommand(int command);
	void initData();
    void receiveData(int byteCount);
	void sendData();
	
  private:
    // properties
    int address;
    int response;
    SMBusCommand_t commands[MAX_COMMANDS];
};

#endif
