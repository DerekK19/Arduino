#include <VirtualWire.h>

// Number of minutes between posts to receiver
#define POST_SECONDS 10

// PIN for busy LED
#define BUSY_PIN 13

// PIN for transmitting to server
#define TX_PIN 11

//#define FLASH_LED 1

const char *msg = "100";
int minuteCounter = POST_SECONDS;

void setup()
{
  // start serial port
  Serial.begin(9600);
  Serial.println("RF Test App");

  pinMode (TX_PIN, OUTPUT);
  pinMode (BUSY_PIN, OUTPUT);
  
  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_tx_pin(TX_PIN);
  vw_setup(2000);	 // Bits per sec
}

void loop()
{
  // Wait a second (1000 ms)
  delay(1000);

  // Increment our minute timer and return if it's not time to post
  if (++minuteCounter < POST_SECONDS) return;
  minuteCounter = 0;
  
#ifdef FLASH_LED
  digitalWrite(BUSY_PIN, HIGH); // Flash a light to show transmitting
#endif

  char buf[128];
  sprintf(buf, "TST %ld", millis());
  vw_send((uint8_t *)buf, strlen(buf));
  vw_wait_tx(); // Wait until the whole message is gone

#ifdef FLASH_LED
  digitalWrite(BUSY_PIN, LOW);
#endif
}
