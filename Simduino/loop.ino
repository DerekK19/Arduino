/* Flashing LED
*  ------------
*
* turns on and off a set of light emitting diodes connected to
* digital pins 2 - 13

* Created 3 February 2012
* Derek Knight
*/


void setup()
{
// Set digital pins 6-13 as outputs
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
}

void loop()
{
  for (int ledPin=6; ledPin < 14; ledPin++)
  {
    delay(600);                 // waits for a while
    digitalWrite(ledPin, HIGH); // sets the LED on
    delay(600);                 // waits for a while
    digitalWrite(ledPin, LOW);  // sets the LED off
  }
}