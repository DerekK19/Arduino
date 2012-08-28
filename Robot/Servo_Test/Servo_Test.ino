// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.

#include <Servo.h> 
#include <LiquidCrystal.h>

#define LCD1 12
#define LCD2 11
#define LCD3 7
#define LCD4 6
#define LCD5 5
#define LCD6 4

#define SWITCH_PIN 13
#define XSERVO_PIN 8
#define YSERVO_PIN 10

#define RANGE_PIN A0

#define OUTA_LEFT 12
#define OUTB_LEFT 11
#define OUTA_RIGHT 6
#define OUTB_RIGHT 7
//#define BIN1 5
//#define AIN1 4
//#define BIN2 6
//#define AIN2 7

#define SR_DATA 2
#define SR_LATCH 4
#define SR_CLOCK 3

#define SR_PWMA 1
#define SR_AIN2 2
#define SR_AIN1 3
#define SR_STBY 4
#define SR_BIN1 5
#define SR_BIN2 6
#define SR_PWMB 7

#define ADVANCE 1
#define REVERSE 2
#define STOP 0

void testMotors();
void motor(int motorA, int motorB);
bool pan();

// initialize the library with the numbers of the interface pins
// LiquidCrystal lcd(LCD1, LCD2, LCD3, LCD4, LCD5, LCD6);

int sensorPin = RANGE_PIN;

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

// Function run once on startup
void setup() 
{
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Robot test system");
  delay(5000);
  
  pinMode (RANGE_PIN, INPUT);

  pinMode (SR_DATA, OUTPUT);
  pinMode (SR_LATCH, OUTPUT);
  pinMode (SR_CLOCK, OUTPUT);
  
  xServo.attach(XSERVO_PIN);  // attaches the servo on XSERVO_PIN to the servo object 
  yServo.attach(YSERVO_PIN);  // attaches the servo on YSERVO_PIN to the servo object 
  x = -1;
  y = -1;
  run = 0;
  toggle = 0;
  
  nearX = startX;
  nearY = startY;

  testMotors();
  
//  lcd.begin(16, 2);
//  lcd.clear();
} 
 
// Function runs continuously
void loop() 
{
  // Check for the toggle switch. If it changes from Low to High, pause or resume running
  if (digitalRead(SWITCH_PIN) == LOW)
  {
    toggle = 1;
  }
  else if (toggle == 1)
  {
    toggle = 0;
    run = 1-run;
//    if (run == 0) lcd.clear();
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
//    lcd.clear();
    return;
  }
  
  int d = distance();
  
  if (d > 0 && d < near)
  {
    near = d;
    nearX = x;
    nearY = y;
  }
  
//  display(x, y, d, near, nearX, nearY, rangeX, rangeY);  
  
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

// Rotate the motors
void motor(int motorA, int motorB)
{
	digitalWrite(SR_LATCH, LOW);
	int value = 1;										// For PWMA
	value = value * 2 + (motorA == REVERSE ? 1 : 0);	// For AIN2
	value = value * 2 + (motorA == ADVANCE ? 1 : 0);	// For AIN1
	value = value * 2 + 1;								// For STBY
	value = value * 2 + (motorB == ADVANCE ? 1 : 0);	// For BIN1
	value = value * 2 + (motorB == REVERSE ? 1 : 0);	// For BIN2
	value = value * 2 + 1;								// For PWMB
	value = value * 2;									// For unused bit 8
//	Serial.println(value);
	shiftOut(SR_DATA, SR_CLOCK, MSBFIRST, value);
	digitalWrite(SR_LATCH, HIGH);
}

void testMotors()
{
#define RUN 150
#define PAUSE 1000
#define WAIT 2000
  Serial.println("Right forward");
  motor(ADVANCE, STOP);
  delay(RUN);
  motor(STOP, STOP);
  delay(PAUSE);
  Serial.println("Left forward");
  motor(REVERSE, STOP);
  delay(RUN);
  motor(STOP, STOP);
  delay(WAIT);
  Serial.println("Right forward");
  motor(STOP, ADVANCE);
  delay(RUN);
  motor(STOP, STOP);
  delay(PAUSE);
  Serial.println("Right backward");
  motor(STOP, REVERSE);
  delay(RUN);
  motor(STOP, STOP);
  delay(WAIT);
  Serial.println("Both forward");
  motor(ADVANCE, ADVANCE);
  delay(RUN);
  motor(STOP, STOP);
  delay(PAUSE);
  Serial.println("Both backward");
  motor(REVERSE, REVERSE);
  delay(RUN);
  motor(STOP, STOP);
  
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
/*
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
*/
}
