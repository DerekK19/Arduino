
// include the library code:
#include <SPI.h>
#include <util.h>
#include <VirtualWire.h>
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define ERROR_PIN 6
#define BUSY_PIN 7
#define RX_PIN 8

float temperature = 0.0;
float humidity = 0.0;

int tick=0;

int bps = 4800;

int gotACK = 1;

unsigned long last = millis();

void setup()
{
  // start serial port
  Serial.begin(9600);
  Serial.println("RF Receive Test");

  pinMode(RX_PIN, INPUT);
  pinMode(BUSY_PIN, OUTPUT);     
  pinMode(ERROR_PIN, OUTPUT);     

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("temp");
  lcd.setCursor(0, 1);
  lcd.print("humidity");
  
  initData();
}

void loop()
{
  if (millis()-last > 20000)
  {
    last = millis();
  	bps += 100;  	
  	Serial.print("Trying ");
  	Serial.print(bps);
  	Serial.println(" bps");
  	vw_rx_stop();
  	initData();
  }
  getData();
}

void initData()
{
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_rx_pin(RX_PIN);
  vw_setup(bps);	 // Bits per sec
  vw_rx_start();       // Start the receiver PLL running
  delay(1000);
}

void getData()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) // Non-blocking
  {
    if (++tick >= 2) tick = 0;
    //Serial.println();
    Serial.print("Got >|");
    for (int i = 0; i < buflen; i++)
    {
      Serial.print(((char)buf[i]));
    }
    Serial.println("|<");
    int i;
    int val;

    digitalWrite(BUSY_PIN, true); // Flash a light to show received good message
    
    // The first 3 characters are the service code:
    // WEA is a weather reading
    if (buf[0] == 'W' && buf[1] == 'E' && buf[2] == 'A')
    {
      // Weather readings are of the form "WEA ttt.t hhh.h"
      // where ttt.t is the temperature in degress celsius and 0hhh.h is the relative humidity
      char carray[5];
      for (int i=0; i< 5; i++) carray[i] = buf[i+4];
      temperature = atof(carray);
      for (int i=0; i< 5; i++) carray[i] = buf[i+9];
      humidity = atof(carray);
    
      lcd.setCursor(9, 0);
      lcd.print(temperature);
      lcd.setCursor(9, 1);
      lcd.print(humidity);
      
//      client = server.available();
//      if (client.connect(service,80)) {
//        client.print("GET /weather/service/data/add?temperature=");
//        client.print(temperature);
//        client.print("&humidity=");
//        client.print(humidity);
//        client.println();
//        gotACK = 0;
//      }
      delay(1000);
    }
    else
    {
      Serial.println("Error");
      digitalWrite(ERROR_PIN,true);
      delay(1000);
      digitalWrite(ERROR_PIN,false);
    }

    digitalWrite(BUSY_PIN, false);
  }
}

