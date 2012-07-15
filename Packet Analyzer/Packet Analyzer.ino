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
 
void writeBuffer();

int incr = 5;
int low = 100;
int high = 200;
int count = 0;
int repeat = 4;
int dms = low;

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
  unsigned char tempByte; //Temporary Byte
 
//  Serial.print(PORTB, HEX);
//  Serial.print(PINB, HEX);
//  Serial.print(" ");
//  if (PORTB & 0x01) return;
  //while (PORTB & 0x01); //Not functioning as expected
  while (digitalRead(8)); //While HIGH idle
 
  //LOW transmitted enter For Loop
  //Repeat loop for each byte in bufferLength
  for (unsigned char b = 0; b < bufferLength; b++)
  {
    tempByte = 0; //Reset temporary byte
 
    //Loop for each bit
    for (char bit = 0; bit < 8; bit++)
    {
      tempByte = tempByte << 1; //Shift previous bit
      tempByte = tempByte & 0xfe;
      //tempByte = tempByte | (PORTB & 0x01); //Not functioning as expected
      tempByte = tempByte | (digitalRead(8)); //Incoming value
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
  if (dms > high) incr = -1;
}

void writeBuffer()
{
  //Write buffer to Serial connection
  Serial.print("SOH ");
  Serial.print(dms);
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