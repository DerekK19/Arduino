#include <DHT22.h>
#include <VirtualWire.h>

// Number of minutes between posts to receiver
#define POST_MINUTES 5

// PIN for busy LED
#define BUSY_PIN 13

// PIN for transmitting to server
#define TX_PIN 11

//#define FLASH_LED 1
//#define TEST_MODE 1
//#define SERIAL_DISPLAY 1

// Data wire for sesnsor is plugged into port 7 on the Arduino
// Connect a 4.7K resistor between VCC and the data pin (strong pullup)
#define DHT22_PIN 7

// Setup a DHT22 instance
DHT22 myDHT22(DHT22_PIN);

const char *msg = "100";
int minuteCounter = POST_MINUTES;

void setup()
{
  Serial.begin(9600);
#ifdef SERIAL_DISPLAY
  // start serial port
  Serial.println("DHT22 Weather sender");
#endif

  pinMode (TX_PIN, OUTPUT);
  pinMode (BUSY_PIN, OUTPUT);
  
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_tx_pin(TX_PIN);
  vw_setup(2000);	 // Bits per sec
  
  // The sensor requires a minimum 2s warm-up after power-on
  delay(5000);
}

void loop()
{
  DHT22_ERROR_t errorCode;
  
  // The sensor can only be read from every 1-2s, and requires a minimum
  // 2s warm-up after power-on. However we will only read every POST_MINUTES
  // minutes so it's no biggy
  
#ifdef TEST_MODE
// In test mode, just wait 5 seconds
  delay(5000);
#else
  // Wait a minute (60,000 ms)
  delay(10000);
  delay(10000);
  delay(10000);
  delay(10000);
  delay(10000);
  delay(10000);
  
  // Increment our minute timer and return if it's not time to post
  if (++minuteCounter < POST_MINUTES) return;
  minuteCounter = 0;
#endif
  
#ifdef FLASH_LED
  digitalWrite(BUSY_PIN, HIGH); // Flash a light to show transmitting
#endif

  errorCode = myDHT22.readData();
  switch(errorCode)
  {
    case DHT_ERROR_NONE:
      char buf[128];
      sprintf(buf, "WEA %02hi.%01hi %02hi.%01hi",
                   myDHT22.getTemperatureCInt()/10, abs(myDHT22.getTemperatureCInt()%10),
                   myDHT22.getHumidityInt()/10, myDHT22.getHumidityInt()%10);
#ifdef SERIAL_DISPLAY
      Serial.println(buf);
#endif
#ifndef TEST_MODE
      vw_send((uint8_t *)buf, strlen(buf));
      vw_wait_tx(); // Wait until the whole message is gone
#endif
      break;
    case DHT_ERROR_CHECKSUM:
      Serial.print("check sum error ");
      Serial.print(myDHT22.getTemperatureC());
      Serial.print("C ");
      Serial.print(myDHT22.getHumidity());
      Serial.println("%");
      break;
    case DHT_BUS_HUNG:
      Serial.println("BUS Hung ");
      break;
    case DHT_ERROR_NOT_PRESENT:
      Serial.println("Not Present ");
      break;
    case DHT_ERROR_ACK_TOO_LONG:
      Serial.println("ACK time out ");
      break;
    case DHT_ERROR_SYNC_TIMEOUT:
      Serial.println("Sync Timeout ");
      break;
    case DHT_ERROR_DATA_TIMEOUT:
      Serial.println("Data Timeout ");
      break;
    case DHT_ERROR_TOOQUICK:
      Serial.println("Polled to quick ");
      break;
  }

#ifdef FLASH_LED
  digitalWrite(BUSY_PIN, LOW);
#endif
}
