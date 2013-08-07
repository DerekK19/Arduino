/*
=================================================================================
 Name        : SMBusSlave.cpp
 Version     : 0.1

 Description :
     Commander library for Arduino as i2c-slave for processing commands/actions.

=================================================================================
*/

#define DEBUG FALSE

#include "Arduino.h"
#include "Wire.h"
#include "SMBusSlave.h"

// constructor:
SMBusSlave::SMBusSlave(int address)
{
  this->address = address;
  for (int i = 0; i < MAX_COMMANDS; i++) {
    this->commands[i].isActive = false;
  }
}

// start Wire processing
void SMBusSlave::startProcessing(void (*receiveData)(int), void (*sendData)())
{
  // initialize i2c as slave
  Wire.begin(this->address);   
  
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);
}

// add command method:
void SMBusSlave::addCommand(int command, int (*callback)(int), int arg)
{
  for (int i = 0; i < MAX_COMMANDS; i++)
  {
    SMBusCommand_t *aCommand = &(this->commands[i]);
    if (!aCommand->isActive)
    {
      aCommand->command = command;
      aCommand->callback = callback;
      aCommand->arg = arg;
      aCommand->isActive = true;
      break;
    }
  }
}

// process command method:
int SMBusSlave::processCommand(int command)
{
  for (int i = 0; i < MAX_COMMANDS; i++) 
  {
    SMBusCommand_t *aCommand = &(this->commands[i]);
    if (aCommand->isActive)
    { 
      if (aCommand->command == command)
      {
        // call callback-function
        this->response = aCommand->callback(aCommand->arg);
        return 1;
      }
    }
  }
  this->response = 255;
  return 0;
}

void SMBusSlave::initData()
{
    this->response = 0;
}

// receive data from the Wire library
void SMBusSlave::receiveData(int byteCount) 
{
  int inputValue = Wire.read();

  this->processCommand(inputValue);
}

// Send data to the Wire library
void SMBusSlave::sendData()
{
    Wire.write(this->response);  
    this->response = 0;
}


