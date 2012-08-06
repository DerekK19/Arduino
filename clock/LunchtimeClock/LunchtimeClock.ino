#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68

int clockpin1 = 10;
int clockpin2 = 11;

// These set the strength of the force that will be applied to the clock electro-magnet.
// Small numbers make the tick quieter, but introduce an oscillation whistle
int strength1 = 250; //130;
int strength2 = 250; // Any lower and we get a whistle from the mechanism

bool nearlyLunchtime();
void TurnTurnTurn(int timeToWait);
void digitalTTurnTurnTurn(int timeToWait);
byte decToBcd(byte val);
byte bcdToDec(byte val);
void getDateDs1307(byte *second,
          byte *minute,
          byte *hour,
          byte *dayOfWeek,
          byte *dayOfMonth,
          byte *month,
          byte *year);

void setup()
{
  Serial.begin(9600);
  Serial.println("Lunchtime Clock sketch");
  pinMode (clockpin1, OUTPUT);
  pinMode (clockpin2, OUTPUT);
  Wire.begin();
}

void loop()
{
  TurnTurnTurn(1000);

  if (nearlyLunchtime()) {
    /*
     * It's now 11am.
     * Make each "second" last only 800 millisecs. Do this for 1800 "seconds" i.e 48 minutes
     */
    for (int i = 0; i < 1800; i++)
    {
      TurnTurnTurn(800);
    }
    /*
     * Then make each "second" last 1200 millisecs. Do this for 1800 "seconds" i.e 72 minutes
     */		
    for (int i = 0; i < 1800; i++)
    {
      TurnTurnTurn(1200);
    }
    /* And we will now be at 1pm. */
  }
}

bool nearlyLunchtime()
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  Serial.print(hour, DEC);
  Serial.print(":");
  Serial.print(minute, DEC);
  Serial.print(":");
  Serial.print(second, DEC);
  Serial.print("  ");
  Serial.print(month, DEC);
  Serial.print("/");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(year, DEC);
  Serial.print("  Day_of_week:");
  Serial.println(dayOfWeek, DEC);

  return hour == 11;
}
/*
 * Perform two tick cycles.
 * Generally the time to wait is 1000 millisecs, so this would cover 2 seconds
 * I found if clock pin 2 is set to a PWM value < 255, the mechanism will whistle
 */
void TurnTurnTurn(int timeToWait)
{
  analogWrite(clockpin1, 0);
  analogWrite(clockpin2, strength2);
  delay(timeToWait);
  analogWrite(clockpin2, 0);
  analogWrite(clockpin1, strength1);
  delay(timeToWait);
}

void digitalTurnTurnTurn(int timeToWait)
{
  digitalWrite(clockpin1, LOW);
  digitalWrite(clockpin2, HIGH);
  delay(timeToWait);
  digitalWrite(clockpin2, LOW);
  digitalWrite(clockpin1, HIGH);
  delay(timeToWait);
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

