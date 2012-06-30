/*
Range finder test, with LCD output
 */

#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int sensorPin = A0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  Serial.begin(9600);
  Serial.println("Range finder Test");
  // declare the ledPin as an OUTPUT:
  //pinMode(ledPin, OUTPUT);  
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);    

#define VOLTS_PER_UNIT    .0049F        // (.0049 for 10 bit A-D)
float volts;
float inches;
float cm;

  volts = (float)sensorValue * VOLTS_PER_UNIT; // ("proxSens" is from analog read)
  inches = 23.897 * pow(volts,-1.1907); //calc inches using "power" trend line from Excel
  cm = 60.495 * pow(volts,-1.1904);     // same in cm
  if (volts < .2) { inches = -1.0; cm = -1.0; }        // out of range    

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);  
  lcd.print (cm);  
  Serial.println (cm);
  
  delay (1000);
}
