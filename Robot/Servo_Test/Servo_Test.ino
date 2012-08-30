// Sweep
// by BARRAGAN <http://barraganstudio.com> 
// This example code is in the public domain.

#include <ServoTimer2.h>
#include <VirtualWire.h>

#define DEBUG TRUE
#define DIAG FALSE

#if DEBUG == TRUE
#define debugBegin()		Serial.begin(9600)
#define debugWrite(x)		Serial.write(x)
#define debugPrint(x)		Serial.print(x)
#define debugPrintln		Serial.println
#define debugPrintDECln(x)	Serial.println(x, DEC)
#define debugPrintHEXln(x)	Serial.println(x, HEX)
#else
#define debugBegin()
#define debugWrite(x)
#define debugPrint(x)
#define debugPrintln
#define debugPrintDECln(x)
#define debugPrintHEXln(x)
#endif

#define SWITCH_PIN 13
#define XSERVO_PIN 8
#define YSERVO_PIN 10

#define RANGE_PIN A5

#define OUTA_LEFT 12
#define OUTB_LEFT 11
#define OUTA_RIGHT 3
#define OUTB_RIGHT 2

#define SR_DATA 5
#define SR_LATCH 6
#define SR_CLOCK 7

#define VW_PIN 4

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

#define VW_SPEED 2000
#define DEBUG_SPEED 9600

#define MIN_PULSE  750
#define MAX_PULSE  2250

#define DEGREES_TO_PW(v) (map(v, 0, 180, MIN_PULSE, MAX_PULSE))

void testMotors();
void motor(int motorA, int motorB);
bool pan();
void diagSetup();
void diagWrite(int x, int y, char *t);
void diagPrint(int x, int y, int i);
void diagDisplay(int x, int y, int d, int near, int nearX, int nearY, int rangeX, int rangeY);

int sensorPin = RANGE_PIN;

ServoTimer2 xServo;  // create servo object to control a servo 
               		 // a maximum of eight servo objects can be created 
ServoTimer2 yServo;  // create servo object to control a servo 
               		 // a maximum of eight servo objects can be created 
Encoder leftEnc (OUTA_LEFT, OUTB_LEFT);
Encoder rightEnc (OUTA_RIGHT, OUTB_RIGHT);
 
int run = 0;
int toggle = 0;

int x = 0;     		 // variables to store the servo positions 
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
  debugBegin();
  debugPrintln("");
  debugPrintln("Robot test system");
  
  pinMode (RANGE_PIN, INPUT);

  pinMode (SR_DATA, OUTPUT);
  pinMode (SR_LATCH, OUTPUT);
  pinMode (SR_CLOCK, OUTPUT);
  
  pinMode (VW_PIN, OUTPUT);
  
  leftEnc.start();
  rightEnc.start();

  delay(5000);
  
  diagSetup();
  diagClear();
  
  xServo.attach(XSERVO_PIN);  // attaches the servo on XSERVO_PIN to the servo object 
  yServo.attach(YSERVO_PIN);  // attaches the servo on YSERVO_PIN to the servo object 
  
//  enc.init (OUTA_LEFT, OUTB_LEFT, OUTA_RIGHT, OUTB_RIGHT);
  
  x = -1;
  y = -1;
  run = 0;
  toggle = 0;
  
  nearX = startX;
  nearY = startY;

  testMotors();  
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
    if (run == 0) diagClear();
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
    diagClear();
    return;
  }
  
  int d = distance();
  
  if (d > 0 && d < near)
  {
    near = d;
    nearX = x;
    nearY = y;
  }
  
  diagDisplay(x, y, d, near, nearX, nearY, rangeX, rangeY);  
  
  if (scanned)
  {
    debugPrint("Near ");
    debugPrint(nearX);
    debugWrite(",");
    debugPrint(nearY);
    debugPrintln("");
    
    if (nearX > 0 && nearY > 0)
    {
      if (rangeX > 1) {rangeX /= 2; if (incrX > 1) incrX--;}
      if (rangeY > 1) {rangeY /= 2; if (incrY > 1) incrY--;}
      speed += 5;
    
      if (nearX < minX+rangeX) nearX = minX + rangeX;
      if (nearX > maxX-rangeX) nearX = maxX - rangeX;
      if (nearY < minY+rangeY) nearY = minY + rangeY;
      if (nearY > maxY-rangeY) nearY = maxY - rangeY;
    
      debugPrint("-> ");
      debugPrint(nearX);
      debugWrite(",");
      debugPrint(nearY);
      debugPrintln("");

      minX = nearX - rangeX;
      maxX = nearX + rangeX;
      minY = nearY - rangeY;
      maxY = nearY + rangeY;
    
      debugPrint("New box ");
      debugPrint(minX);
      debugWrite(",");
      debugPrint(minY);
      debugPrint(" - ");
      debugPrint(maxX);
      debugWrite(",");
      debugPrint(maxY);
      debugPrint(" (");
      debugPrint(rangeX);
      debugWrite(",");
      debugPrint(rangeY);
      debugPrintln(")");
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
	value = (value << 1) + (motorA == REVERSE ? 1 : 0);	// For AIN2
	value = (value << 1) + (motorA == ADVANCE ? 1 : 0);	// For AIN1
	value = (value << 1) + 1;							// For STBY
	value = (value << 1) + (motorB == ADVANCE ? 1 : 0);	// For BIN1
	value = (value << 1) + (motorB == REVERSE ? 1 : 0);	// For BIN2
	value = (value << 1) + 1;							// For PWMB
	value = (value << 1);								// For unused bit 8
	debugPrintHEXln(value);
	shiftOut(SR_DATA, SR_CLOCK, MSBFIRST, value);
	digitalWrite(SR_LATCH, HIGH);
}

void testMotors()
{
#define RUN 150
#define PAUSE 1000
#define WAIT 2000
  delay(WAIT);
  debugPrintDECln(leftEnc.read());
  debugPrintDECln(rightEnc.read());
  debugPrintln("Left forward");
  motor(ADVANCE, STOP);
  delay(RUN);
  debugPrintDECln(leftEnc.read());
  debugPrintDECln(rightEnc.read());
  motor(STOP, STOP);
  delay(PAUSE);
  debugPrintln("Left forward");
  motor(REVERSE, STOP);
  delay(RUN);
  motor(STOP, STOP);
  debugPrintDECln(leftEnc.read());
  debugPrintDECln(rightEnc.read());
  delay(WAIT);
  debugPrintln("Right forward");
  motor(STOP, ADVANCE);
  delay(RUN);
  motor(STOP, STOP);
  delay(PAUSE);
  debugPrintln("Right backward");
  motor(STOP, REVERSE);
  delay(RUN);
  motor(STOP, STOP);
  delay(WAIT);
  debugPrintln("Both forward");
  motor(ADVANCE, ADVANCE);
  delay(RUN);
  motor(STOP, STOP);
  delay(PAUSE);
  debugPrintln("Both backward");
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
  
  int xm = DEGREES_TO_PW(x);    // scale it to use it with the servo (0-180 -> 750-2250) 
  int ym = DEGREES_TO_PW(y);    // scale it to use it with the servo (0-180 -> 750-2250) 
  xServo.write(xm);             // sets the servo position according to the scaled value
  yServo.write(ym);             // sets the servo position according to the scaled value
  delay(speed);                 // waits for the servo to reach the position
  
  debugPrint(x);
  debugWrite(",");
  debugPrint(y);
  debugWrite(" -> ");
  debugPrint(xm);
  debugWrite(",");
  debugPrint(ym);
  debugWrite(" (");
  debugPrint(maxX);
  debugWrite(",");
  debugPrint(maxY);
  debugPrintln(")");
  
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

void diagSetup()
{
//  lcd.begin(16, 2);
#if DIAG
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_tx_pin(VW_PIN);
  vw_setup(VW_SPEED);	 // Bits per sec
  vw_send((uint8_t *)"LCD INIT", 8);
  vw_wait_tx(); // Wait until the whole message is gone
#endif
}

void diagClear()
{
//  lcd.clear();
#if DIAG
  vw_send((uint8_t *)"LCD CLEAR", 9);
  vw_wait_tx(); // Wait until the whole message is gone
#endif
}

void diagWrite(int x, int y, char *t)
{
//  lcd.setCursor(x, y);
//  lcd.write(t);
#if DIAG
  char buf[128];
  sprintf(buf, "LCD OUT %02hi %02hi %s", x, y, t);
  vw_send((uint8_t *)buf, strlen(buf));
  vw_wait_tx(); // Wait until the whole message is gone
#endif
}

void diagPrint(int x, int y, int i)
{
//  lcd.setCursor(x, y);
//  lcd.print(i);
#if DIAG
  char buf[128];
  sprintf(buf, "LCD OUT %02hi %02hi %d", x, y, i);
  vw_send((uint8_t *)buf, strlen(buf));
  vw_wait_tx(); // Wait until the whole message is gone
#endif
}

// Display results
void diagDisplay(int x, int y, int d, int near, int nearX, int nearY, int rangeX, int rangeY)
{
#if DIAG
  diagWrite(0, 0, "    ");
  diagWrite(0, 1, "    ");
  diagWrite(4, 0, "    ");
  diagWrite(4, 1, "    ");
  diagWrite(8, 0, "    ");
  diagWrite(8, 1, "    ");
  diagWrite(12,0, "    ");
  diagWrite(12,1, "    ");
  
  diagPrint(0, 0, x);
  diagPrint(0, 1, y);
  diagPrint(4, 0, nearX);
  diagPrint(4, 1, nearY);
  diagPrint(8, 0, d);
  diagPrint(8, 1, near < 10000 ? near : -1);
  diagPrint(12,0, rangeX);
  diagPrint(12,1, rangeY);
#endif
}
