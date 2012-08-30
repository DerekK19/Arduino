#include <VirtualWire.h>

#define ERROR_PIN 6
#define BUSY_PIN 7
#define RX_PIN 8

void setup()
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Diagnostic receiver");

  pinMode(RX_PIN, INPUT);
  pinMode(BUSY_PIN, OUTPUT);     
  pinMode(ERROR_PIN, OUTPUT);     

  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_rx_pin(RX_PIN);
  vw_setup(2000);	 // Bits per sec

  vw_rx_start();       // Start the receiver PLL running
}

void loop()
{
  getData();
}

void getData()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    //Serial.println();
    Serial.print("Got >|");
    for (int i = 0; i < buflen; i++)
    {
      Serial.print(((char)buf[i]));
    }
    Serial.println("|<");

    digitalWrite(BUSY_PIN, true); // Flash a light to show received good message
    
    // The first 3 characters are the service code:
    // WEA is a weather reading
    if (buf[0] == 'R' && buf[1] == 'B' && buf[2] == 'T')
    {
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
