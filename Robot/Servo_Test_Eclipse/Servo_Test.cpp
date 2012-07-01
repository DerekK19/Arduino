// Do not remove the include below
#include "Servo_Test.h"

// Forward references
bool panNear(int xPos, int yPos);
int distance();
void display(int x, int y, int d, int near, int nearX, int nearY, int rangeX, int rangeY);

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int sensorPin = A0;

Servo xServo;  // create servo object to control a servo
                // a maximum of eight servo objects can be created
Servo yServo;  // create servo object to control a servo
                // a maximum of eight servo objects can be created

int x = 0;    // variables to store the servo positions
int y = 0;
int loX = 30;
int hiX = 150;
int loY = 45;
int hiY = 100;
int incrX = 1;
int incrY = 5;
int startX = (loX+hiX)/2;
int startY = (loY+hiY)/2;
int rangeX = startX - loX;
int rangeY = startY - loY;
int speed = 15;
int near = 10000;
int nearX = 0;
int nearY = 0;

//The setup function is called once at startup of the sketch
void setup()
{
Serial.begin(9600);
xServo.attach(9);  // attaches the servo on pin 11 to the servo object
yServo.attach(10);  // attaches the servo on pin 12 to the servo object
x = -1;
y = -1;

nearX = startX;
nearY = startY;

lcd.begin(16, 2);
}

// The loop function is called in an endless loop
void loop()
{
int scanned = 0;
if (rangeX > 0 || rangeY > 0) scanned = panNear(startX, startY);

int d = distance();

if (d > 0 && d < near)
{
  near = d;
  nearX = x;
  nearY = y;
}

display(x, y, d, near, nearX, nearY, rangeX, rangeY);

if (scanned)
{
  startX = nearX; startY = nearY;
  if (rangeX > 20) rangeX /= 2;
  if (rangeY > 20) rangeY /= 2;
  speed += 5;

  loX = startX - rangeX;
  hiX = startX + rangeX;
  loY = startY - rangeY;
  hiY = startY - rangeY;

  near = 10000;
  nearX = 0;
  nearY = 0;

  x = -1;
  y = -1;

  if (loX < 30) loX = 30;
  if (hiX > 150) hiX = 150;
  if (loY < 45) loY = 45;
  if (hiY > 100) hiY = 100;

  scanned = 0;
}
}

bool panNear(int xPos, int yPos)
{
if (x < 0) x = xPos;
if (y < 0) y = yPos;

x += incrX;
if (x >= hiX || x <= loX) incrX = -incrX;
if (x == xPos)
{
  y += incrY;
  if (y <= loY || y >= hiY) incrY = -incrY;
}
xServo.write(x);              // tell servo to go to position in variable 'pos'
yServo.write(y);              // tell servo to go to position in variable 'pos'
delay(speed);                    // waits 15ms for the servo to reach the position

Serial.print(x);
Serial.write(" ");
Serial.print(xPos);
Serial.write(" ");
Serial.print(y);
Serial.write(" ");
Serial.print(yPos);
Serial.println("");

return x == xPos && y == yPos ? 1 : 0;
}

int distance()
{  // read the value from the sensor:
int sensorValue = analogRead(sensorPin);

#define VOLTS_PER_UNIT    .0049F        // (.0049 for 10 bit A-D)
float volts;
float inches;
float cm;

volts = (float)sensorValue * VOLTS_PER_UNIT; // ("proxSens" is from analog read)
inches = 23.897 * pow(volts,-1.1907); //calc inches using "power" trend line from Excel
cm = 60.495 * pow(volts,-1.1904);     // same in cm
if (volts < .2) { inches = -1.0; cm = -1.0; }        // out of range

return cm;
}

void display(int x, int y, int d, int near, int nearX, int nearY, int rangeX, int rangeY)
{
lcd.setCursor(0, 0);
lcd.write("   ");
lcd.setCursor(0, 1);
lcd.write("   ");
lcd.setCursor(4,0);
lcd.write("   ");
lcd.setCursor(4,1);
lcd.write("   ");
lcd.setCursor(8,0);
lcd.write("   ");
lcd.setCursor(8,1);
lcd.write("   ");
lcd.setCursor(12,0);
lcd.write("   ");
lcd.setCursor(12,1);
lcd.write("   ");

lcd.setCursor(0, 0);
lcd.print(x);
lcd.setCursor(0, 1);
lcd.print(y);
lcd.setCursor(4, 0);
lcd.print(nearX);
lcd.setCursor(4, 1);
lcd.print(nearY);
lcd.setCursor(8,0);
lcd.print(d);
lcd.setCursor(8,1);
lcd.print(near);
lcd.setCursor(12, 0);
lcd.print(rangeX);
lcd.setCursor(12, 1);
lcd.print(rangeY);
}
