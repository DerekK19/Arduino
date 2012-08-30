/**
 * Packet Analyzer
 *
 * Project documented at:
 * http://www.eightlines.com/blog/2009/04/arduino-packet-analyzer/
 *
 * Many thanks to Andrew Kilpatrick:
 * http://andrewkilpatrick.org/blog/
 *
 * Captures & logs incoming packets in HEX format.
 * Digital signal is HIGH and drops to LOW when transmitting.
 * Accepts digital input on Digital Pin 8.
 */
 
unsigned char buff[128]; //Create a buffer array to insert bytes
int bufferLength = 128;
 
void readBuffer();
void writeBuffer();

int low = 4;
int high = 12;
int incr = 1;

int count = 0;
int repeat = 8;
int dms = low;
uint8_t slen = 255;

unsigned long start, end, takenLo[100], takenHi[100];
unsigned int iterLo[100], iterHi[100];
int countLo, countHi;
int hi=0;
int iter;

void setup()
{
  Serial.begin(9600); //Invoke serial connection
  Serial.println("");
  Serial.println("Packet Analyzer");
 
  //Set input based on port registers
  //Refers to Digital Pin 8
  //http://arduino.cc/en/Reference/PortManipulation
  DDRB = DDRB | B11111110; //Data Direction Register
  PORTB = 0xFF; //Port B Register
}
 
void loop()
{
//	studyStream();
    readBuffer();
}

int myDigitalRead(uint8_t pin)
{
	if (*portInputRegister(2) & 1) return HIGH;
	return LOW;
}

#define PIN8_VALUE ((*portInputRegister(2) & 1) == 1 ? HIGH : LOW)
#define PIN8_HIGH ((*portInputRegister(2) & 1) == 1)
#define PIN8_LOW ((*portInputRegister(2) & 1) == 0)

/* --------------------- */
void studyStream()
{
  if (countLo > 80 || countHi > 80)
  {
/*
int pin = 8;
uint8_t timer = digitalPinToTimer(pin);
uint8_t bit = digitalPinToBitMask(pin);
uint8_t port = digitalPinToPort(pin);
volatile uint8_t reg = *portInputRegister(port);

Serial.print("timer ");
Serial.println(timer);
Serial.print("bit ");
Serial.println(bit);
Serial.print("port ");
Serial.println(port);
Serial.print("reg ");
Serial.println(reg);
*/  
    Serial.println("Lo");
	for (int i=0; i<countLo; i++)
	{
		Serial.print(takenLo[i]);
		Serial.print(":");
		Serial.print(iterLo[i]);
		Serial.print(" ");
		if (i % 20 == 19) Serial.println("");
	}
    Serial.println("Hi");
	for (int i=0; i<countHi; i++)
	{
		Serial.print(takenHi[i]);
		Serial.print(":");
		Serial.print(iterHi[i]);
		Serial.print(" ");
		if (i % 20 == 19) Serial.println("");
	}
	countLo = 0;
	countHi = 0;
  }
  if (hi)
  {
	  iter=0;
	  while (PIN8_LOW); 			// Wait for the line to go HIGH
      start=micros();
	  while (PIN8_HIGH);  	// Now time how long it takes to go LOW
	  end=micros();
	  iterLo[countLo]=iter;
	  takenLo[countLo++]=end-start;
  }
  else
  {
	  iter = 0;
	  while (PIN8_HIGH);  			// Wait for the line to go LOW
	  start=micros();
	  while (PIN8_LOW); 		// Now time how long it takes to go HIGH
	  end=micros();
	  iterHi[countLo]=iter;
  	  takenHi[countHi++]=end-start;
  }
  hi = 1-hi;
}

void readBuffer()
{
  unsigned char tempByte; //Temporary Byte
  
  slen = 255;
 
//  Serial.print(PORTB, HEX);
//  Serial.print(PINB, HEX);
//  Serial.print(" ");
//  if (PORTB & 0x01) return;
  //while (PORTB & 0x01); //Not functioning as expected
  while (PIN8_HIGH); //While HIGH idle
 
  //LOW transmitted enter For Loop
  //Repeat loop for each byte in bufferLength
  uint8_t len = 0;
  uint8_t val = PIN8_VALUE;
  for (unsigned char b = 0; b < bufferLength; b++)
  {
    tempByte = 0; //Reset temporary byte
 
    //Loop for each bit
    for (char bit = 0; bit < 8; bit++)
    {
      tempByte <<= 1; //Shift previous bit
      tempByte &= 0xfe;
      //tempByte = tempByte | (PORTB & 0x01); //Not functioning as expected
      tempByte |= PIN8_VALUE; //Incoming value
      if (PIN8_VALUE != val) {val=PIN8_VALUE;if (len<slen)slen=len;len=0;}
      len++;
      delayMicroseconds(dms); //Oversampled delay - can be adjusted based on speed of packet
    }
 
    buff[b] = tempByte; //Write byte to buffer
  }
  writeBuffer();
  if (count++ > repeat)
  {
  	count = 0;
  	dms += incr;
  }
  if (dms < low) incr = 1;
  if (dms > high) incr = -1;}

void writeBuffer()
{
  //Write buffer to Serial connection
  Serial.print("SOH ");
  Serial.print(dms);
  Serial.print(" [");
  Serial.print(slen);
  Serial.print("]");
  Serial.println();
  unsigned char b = 0;
  for (unsigned int i = 0; i < 8; i++)
  {
	unsigned char c = b;
	for (unsigned int j = 0; j < 16; j++)
  	{
		b++;
	    if (buff[b] < 16 ) Serial.print("0");
	    Serial.print(buff[b], HEX);
    	Serial.print(" ");
    }
    Serial.print("|");
	for (unsigned int j = 0; j < 16; j++)
  	{
		c++;
	    if (buff[c] < 27) Serial.print("."); else Serial.write(buff[c]);
    	Serial.print("");
    }
	Serial.println("");
  }
  Serial.println("EOT");
}