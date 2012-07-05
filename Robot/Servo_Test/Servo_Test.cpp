// Do not remove the include below
#include "Servo_Test.h"

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int sensorPin = A0;

Servo xServo;  // create servo object to control a servo
                // a maximum of eight servo objects can be created
Servo yServo;  // create servo object to control a servo
                // a maximum of eight servo objects can be created

int run = 0;
int toggle = 0;

int x = 0;    // variables to store the servo positions
int y = 0;
int loX = 30;
int hiX = 150;
int loY = 50;
int hiY = 100;
int minX = loX;
int minY = loY;
int maxX = hiX;
int maxY = hiY;
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
// Add your initialization code here

	  Serial.begin(9600);

	  pinMode (7, INPUT);

	  xServo.attach(9);  // attaches the servo on pin 11 to the servo object
	  yServo.attach(10);  // attaches the servo on pin 12 to the servo object
	  x = -1;
	  y = -1;
	  run = 0;
	  toggle = 0;

	  nearX = startX;
	  nearY = startY;

	  lcd.begin(16, 2);
	  lcd.clear();
}

// The loop function is called in an endless loop
void loop()
{
	  // Check for the toggle switch. If it changes from Low to High, pause or resume running
	  if (digitalRead(7) == LOW)
	  {
	    toggle = 1;
	  }
	  else if (toggle == 1)
	  {
	    toggle = 0;
	    run = 1-run;
	    if (run == 0) lcd.clear();
	  }
	  if (run == 0) return;

	  int scanned = 0;
	  if (rangeX > 1 || rangeY > 1)
	  {
	    scanned = pan();
	  }
	  else
	  {
	    run = 0;
	    lcd.clear();
	    return;
	  }

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
	    Serial.print("Near ");
	    Serial.print(nearX);
	    Serial.write(",");
	    Serial.print(nearY);
	    Serial.println("");

	    if (nearX > 0 && nearY > 0)
	    {
	      if (rangeX > 1) {rangeX /= 2; if (incrX > 1) incrX--;}
	      if (rangeY > 1) {rangeY /= 2; if (incrY > 1) incrY--;}
	      speed += 5;

	      if (nearX < minX+rangeX) nearX = minX + rangeX;
	      if (nearX > maxX-rangeX) nearX = maxX - rangeX;
	      if (nearY < minY+rangeY) nearY = minY + rangeY;
	      if (nearY > maxY-rangeY) nearY = maxY - rangeY;

	      Serial.print("-> ");
	      Serial.print(nearX);
	      Serial.write(",");
	      Serial.print(nearY);
	      Serial.println("");

	      minX = nearX - rangeX;
	      maxX = nearX + rangeX;
	      minY = nearY - rangeY;
	      maxY = nearY + rangeY;

	      Serial.print("New box ");
	      Serial.print(minX);
	      Serial.write(",");
	      Serial.print(minY);
	      Serial.print(" - ");
	      Serial.print(maxX);
	      Serial.write(",");
	      Serial.print(maxY);
	      Serial.print(" (");
	      Serial.print(rangeX);
	      Serial.write(",");
	      Serial.print(rangeY);
	      Serial.println(")");
	    }

	    near = 10000;
	    nearX = 0;
	    nearY = 0;

	    x = -1;
	    y = -1;

	    if (minX < loX) minX = loX;
	    if (maxX > hiX) maxX = hiX;
	    if (minY < loY) minY = loY;
	    if (maxY > hiY) maxY = hiY;

	    scanned = 0;
	  }
	}

	// Look round the area bounded by minX,minY - maxX,maxY
	// x,y is the current position, it will be incremented by incrX,incrY
	bool pan()
	{
	  if (x < 0) x = minX-incrX;
	  if (y < 0) y = minY-incrY;

	  x += incrX;
	  if (x == maxX && y == maxY) return 1;
	  if (x >= maxX || x <= minX) incrX = -incrX;
	  if (x == minX || x == maxX)
	  {
	    y += incrY;
	    if (y <= minY || y >= maxY) incrY = -incrY;
	  }

	  if (x < minX) x = minX;
	  if (x > maxX) x = maxX;
	  if (y < minY) y = minY;
	  if (y > maxY) y = maxY;

	  xServo.write(x);              // tell X servo to go to position in variable 'x'
	  yServo.write(y);              // tell Y servo to go to position in variable 'y'
	  delay(speed);                 // waits for the servo to reach the position

	  Serial.print(x);
	  Serial.write(",");
	  Serial.print(y);
	  Serial.write(" -> ");
	  Serial.print(maxX);
	  Serial.write(",");
	  Serial.print(maxY);
	  Serial.println("");

	  return x == maxX && y == maxY ? 1 : 0;
	}

	// Read the distance from the sensor to the object it can 'see'
	int distance()
	{
	  int sensorValue = analogRead(sensorPin);

	#define VOLTS_PER_UNIT    .0049F        // (.0049 for 10 bit A-D)
	  float volts;
	  float inches;
	  float cm;

	  volts = (float)sensorValue * VOLTS_PER_UNIT;    // ("proxSens" is from analog read)
	  inches = 23.897 * pow(volts,-1.1907);           //calc inches using "power" trend line from Excel
	  cm = 60.495 * pow(volts,-1.1904);               // same in cm
	  if (volts < .2) { inches = -1.0; cm = -1.0; }   // out of range

	  return cm;
	}

	// Display results
	void display(int x, int y, int d, int near, int nearX, int nearY, int rangeX, int rangeY)
	{
	  lcd.setCursor(0, 0);
	  lcd.write("    ");
	  lcd.setCursor(0, 1);
	  lcd.write("    ");
	  lcd.setCursor(4,0);
	  lcd.write("    ");
	  lcd.setCursor(4,1);
	  lcd.write("    ");
	  lcd.setCursor(8,0);
	  lcd.write("    ");
	  lcd.setCursor(8,1);
	  lcd.write("    ");
	  lcd.setCursor(12,0);
	  lcd.write("    ");
	  lcd.setCursor(12,1);
	  lcd.write("    ");

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
	  lcd.print(near < 10000 ? near : -1);
	  lcd.setCursor(12, 0);
	  lcd.print(rangeX);
	  lcd.setCursor(12, 1);
	  lcd.print(rangeY);
}
