#include <Wire.h>

void setup() 
{ 
  Serial.begin(115200);
  Serial.println();
  Serial.println ("\n\n[Raspberry-Arduino i2c test program]");

  // define slave address (0x2A = 42)
  #define SLAVE_ADDRESS 0x2A

  // initialize i2c as slave
  Wire.begin(SLAVE_ADDRESS);
   
  // define callbacks for i2c communication
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData); 
}

// callback for received data
void receiveData(int byteCount) 
{
	Serial.print("Recv ");
	Serial.print(byteCount);
	Serial.println("");
  //  […]
}

// callback for sending data
void sendData()
{ 
	Serial.println("Send");
	Wire.write("Hello world");
  //  […]
}

void loop()
{
}
