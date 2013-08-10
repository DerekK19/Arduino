// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.

#include <PololuWheelEncoders.h>

#define DEBUG TRUE

#if DEBUG == TRUE
#define debugBegin()		Serial.begin(9600)
#define debugWrite(x)		Serial.write(x)
#define debugPrint(x)		Serial.print(x)
#define debugPrintDEC(x)	Serial.print(x, DEC)
#define debugPrintHEX(x)	Serial.print(x, HEX)
#define debugPrintln		Serial.println
#define debugPrintDECln(x)	Serial.println(x, DEC)
#define debugPrintHEXln(x)	Serial.println(x, HEX)
#else
#define debugBegin()
#define debugWrite(x)
#define debugPrint(x)
#define debugPrintDEC(x)
#define debugPrintHEX(x)
#define debugPrintln
#define debugPrintDECln(x)
#define debugPrintHEXln(x)
#endif

#define OUTA_LEFT 12
#define OUTB_LEFT 11
#define OUTA_RIGHT 2
#define OUTB_RIGHT 3

#define VW_SPEED 2000
#define DEBUG_SPEED 9600

void debugPrintEncoders();

//Encoder leftEnc (OUTA_LEFT, OUTB_LEFT);
//Encoder rightEnc (OUTA_RIGHT, OUTB_RIGHT);
PololuWheelEncoders encoder;
 
// Function run once on startup
void setup() 
{
  debugBegin();
  debugPrintln("");
  debugPrintln("Robot test system");  

//  leftEnc.start();
//  rightEnc.start();
  encoder.init (OUTA_LEFT, OUTB_LEFT, OUTA_RIGHT, OUTB_RIGHT);

//  leftEnc.write(0);
//  rightEnc.write(0);
} 
 
// Function runs continuously
void loop() 
{
	delay(1000);
	debugPrintEncoders();
}

void debugPrintEncoders()
{
  Serial.print( encoder.getCountsM1(), DEC );
//  debugPrintDEC(leftEnc.read());
  Serial.print(" ");
  Serial.print( encoder.getCountsM2(), DEC );
//  debugPrintDEC(rightEnc.read());
  Serial.print("  ");
//  Serial.print( encoder.checkErrorM1(), DEC );
  Serial.print(" ");
//  Serial.print( encoder.checkErrorM2(), DEC );
  Serial.println();
}
