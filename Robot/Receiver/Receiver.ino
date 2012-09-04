#include <VirtualWire.h>
#include <LiquidCrystal.h>

#define DEBUG FALSE

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

#define ERROR_PIN 8
#define BUSY_PIN 9
#define RX_PIN 13

#define VW_SPEED 5000
#define DEBUG_SPEED 9600

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

void setup()
{
  // start serial port
  debugBegin();
  debugPrintln();
  debugPrintln("Diagnostic receiver");

  pinMode(RX_PIN, INPUT);
  pinMode(BUSY_PIN, OUTPUT);     
  pinMode(ERROR_PIN, OUTPUT);     

  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_rx_pin(RX_PIN);
  vw_setup(VW_SPEED);	 	 // Bits per sec
  vw_rx_start();       		 // Start the receiver PLL running
}

void loop()
{
  getData();
}

void getData()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  char msg[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
//    debugPrint("Got >|");
    for (int i = 0; i < buflen; i++)
    {
      debugPrint(((char)buf[i]));
    }
    debugPrintln("|<");

    digitalWrite(BUSY_PIN, true); // Flash a light to show received good message
    
    // The first 3 characters are the service code:
    // WEA is a weather reading
    if (buf[0] == 'L' && buf[1] == 'C' && buf[2] == 'D')
    {
        debugPrintln("---");
    	if (buflen == 8 && buf[3] == ' ' && buf[4] == 'I' && buf[5] == 'N' && buf[6] == 'I' && buf[7] == 'T')
    	{
    	  debugPrintln("init");
		  lcd.begin(16, 2);
    	}
    	if (buflen == 9 && buf[3] == ' ' && buf[4] == 'C' && buf[5] == 'L' && buf[6] == 'E' && buf[7] == 'A' && buf[8] == 'R')
    	{
    	  debugPrintln("clear");
          lcd.clear();
    	}
    	if (buflen > 14 && buf[3] == ' ' && buf[4] == 'O' && buf[5] == 'U' && buf[6] == 'T' && buf[7] == ' ')
    	{
	  	  int col = ((buf[8]-'0')*10) + (buf[9]-'0');
    	  int row = ((buf[11]-'0')*10) + (buf[12]-'0');
		  buf[buflen]='\0';
		  for (int i = 14; i <= buflen; i++) msg[i-14] = buf[i];
		  lcd.setCursor(col, row);
		  lcd.print(msg);
		  debugPrint(row);
		  debugPrint(" ");
		  debugPrint(col);
		  debugPrint(" |");
		  debugPrint(msg);
		  debugPrintln("|");
    	}
    }
    else
    {
      digitalWrite(ERROR_PIN,true);
      delay(1000);
      digitalWrite(ERROR_PIN,false);
    }

    digitalWrite(BUSY_PIN, false);
  }
}
