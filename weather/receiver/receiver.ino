#include <SPI.h>
//#include <Dhcp.h>
//#include <Dns.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>

// include the library code:
#include <VirtualWire.h>

#define ERROR_PIN 9
#define BUSY_PIN 7
#define RX_PIN 8

// assign a MAC address for the ethernet controller.
// fill in your address here:
// This MAC address is for the Freetronics shield, which doesn't have an assigned MAC address
// I took the Arduino Shield's address and incrmented each word by 1!
byte mac[] = { 
  0x90, 0xA3, 0xDB, 0x04, 0x01, 0x28};
// assign an IP address for the controller:
IPAddress ip(192,168,0,90);
IPAddress gateway(192,168,0,1);	
IPAddress subnet(255, 255, 255, 0);
IPAddress service(192,168,0,9);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(8080);
EthernetClient client;

float temperature = 0.0;
float humidity = 0.0;

int tick=0;

int gotACK = 1;

void setup()
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Weather station base");

  pinMode(RX_PIN, INPUT);
  pinMode(BUSY_PIN, OUTPUT);     
  pinMode(ERROR_PIN, OUTPUT);     

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  //client = server.available();

  // Initialise the IO and ISR
  vw_set_ptt_inverted(true); // Required for DR3100
  vw_set_rx_pin(RX_PIN);
  vw_setup(2000);	 // Bits per sec

  vw_rx_start();       // Start the receiver PLL running
  
  // give the Ethernet shield time to set up:
  delay(1000);
}

void loop()
{
  getData();

  // listen for incoming Ethernet connections:
  listenForEthernetClients();
  
  // Handle any response to the requests made to the server
  if (gotACK == 0)
  {
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
     }
    
     if (!client.connected()) {
       Serial.println("");
       client.stop();
       gotACK = 1;
     }
  }
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
    
      client = server.available();
      if (client.connect(service,80)) {
        client.print("GET /weather/service/data/add?temperature=");
        client.print(temperature);
        client.print("&humidity=");
        client.print(humidity);
        client.println();
        gotACK = 0;
      }
      delay(1000);
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

void listenForEthernetClients() {
  // listen for incoming clients
  // At the moment I don't look at the inoming request, I just treat all requests as a "Current Weather" query
  EthernetClient inClient = server.available();
  if (inClient) {
    Serial.println("Got a client request");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (inClient.connected()) {
      if (inClient.available()) {
        char c = inClient.read();
        Serial.print(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          Serial.println("");
          // send a standard http response header
          inClient.println("HTTP/1.1 200 OK");
          inClient.println("Content-Type: text/html");
          inClient.println();
          // print the current readings, in JSON format:
          inClient.print("{\"Temperature\":\"");
          inClient.print(temperature);
          inClient.print("\",\"Humidity\":\"");
          inClient.print(humidity);
          inClient.println("\"}");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    inClient.stop();
  }
}

