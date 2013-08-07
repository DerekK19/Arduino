/* Flashing LED
*  ------------
*
* turns on and off a set of light emitting diodes connected to
* digital pins 2 - 13

* Created 3 February 2012
* Derek Knight
*/

int ledPin1 = 6;     // LED connected to digital pin 6
int ledPin2 = 7;     // LED connected to digital pin 7
int ledPin3 = 8;     // LED connected to digital pin 8
int ledPin4 = 9;     // LED connected to digital pin 9
int ledPin5 = 10;    // LED connected to digital pin 10
int ledPin6 = 11;    // LED connected to digital pin 11
int ledPin7 = 12;    // LED connected to digital pin 12
int ledPin8 = 13;    // LED connected to digital pin 13
int incr = 0;

void setup()
{
// Set digital pins 6-13 as outputs
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
  pinMode(ledPin6, OUTPUT);
  pinMode(ledPin7, OUTPUT);
  pinMode(ledPin8, OUTPUT);
}

void loop()
{
  int ledPin;
  ledPin = ledPin1;
  if (incr == 1) ledPin = ledPin2;
  if (incr == 2) ledPin = ledPin3;
  if (incr == 3) ledPin = ledPin4;
  if (incr == 4) ledPin = ledPin5;
  if (incr == 5) ledPin = ledPin6;
  if (incr == 6) ledPin = ledPin7;
  if (incr == 7) ledPin = ledPin8;
  delay(600);                 // waits for a while
  digitalWrite(ledPin, HIGH); // sets the LED on
  delay(600);                 // waits for a while
  digitalWrite(ledPin, LOW);  // sets the LED off
  incr = incr + 1;
  incr = incr % 8;
}