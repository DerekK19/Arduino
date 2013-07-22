#include <SPI.h>
//#include <Dhcp.h>
//#include <Dns.h>
#include <Ethernet.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <util.h>

// include the library code:
#include <VirtualWire.h>

#define BUSY_PIN 7
#define ERROR_PIN 8
#define RX_PIN 9

#define DATA_SPEED 2000

// assign a MAC address for the ethernet controller.
// fill in your address here:
// This MAC address is for the Freetronics shield, which doesn't have an assigned MAC address
// I took the Arduino Shield's address and incrmented each word by 1!
byte mac[] = { 
  0x90, 0xA3, 0xDB, 0x04, 0x01, 0x28};
// assign an IP address for the controller:
IPAddress server_ip(192,168,0,90);	// Arduino's ip address
IPAddress station_ip(192,168,0,41); // Niobium - Weather station receiver
IPAddress gateway_ip(192,168,0,1);	// Router gateway
IPAddress subnet_mask(255,255,255,0);
IPAddress service_ip(192,168,0,9);	// Carrington - application web server

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(8080);
EthernetClient wsClient;
EthernetClient inClient;
EthernetClient brClient;

float temperature = 0.0;
float humidity = 0.0;

int service_port=80;
int station_port=8040;

char brJSON[512];
int nJSON;

int tick=0;

int gotACK = 1;
int gotBACK = 1;

void getData();
void listenForEthernetClients();
boolean bridged(String request);

void setup()
{
	// start serial port
	Serial.begin(9600);
	Serial.println("\rWeather station base");

	pinMode(RX_PIN, INPUT);
	pinMode(BUSY_PIN, OUTPUT);     
	pinMode(ERROR_PIN, OUTPUT);     

	// start the Ethernet connection and the server:
	Ethernet.begin(mac, server_ip);
	server.begin();

	// Initialise the IO and ISR
	vw_set_ptt_inverted(true); // Required for DR3100
	vw_set_rx_pin(RX_PIN);
	vw_setup(DATA_SPEED);	 // Bits per sec

	vw_rx_start();       // Start the receiver PLL running

	digitalWrite(BUSY_PIN, true);
	digitalWrite(ERROR_PIN, true);

	// give the Ethernet shield time to set up:
	delay(1000);

	digitalWrite(BUSY_PIN, false);
	digitalWrite(ERROR_PIN, false);
}

void loop()
{
	getData();

	// listen for incoming Ethernet connections:
	listenForEthernetClients();

	// Handle any response to the requests made to the server
	if (gotACK == 0)
	{
		if (wsClient.available()) {
			char c = wsClient.read();
		}
		if (!wsClient.connected()) {
			wsClient.stop();
			gotACK = 1;
		}
	}
	// Handle any response to the requests made to the bridged server
	if (gotBACK == 0)
	{
		if (brClient.available()) {
			if (nJSON < 512) brJSON[nJSON++] = brClient.read();
		}
		if (!brClient.connected()) {
			brJSON[nJSON] = '\0';
			inClient.println("HTTP/1.1 200 OK");
			inClient.println("Content-Type: text/json");
			inClient.println();
			boolean copy = false;
			char *pJSON = brJSON;
			do {
				if (*pJSON == '{') copy = true;
				if (copy) inClient.write(*pJSON);
				if (*pJSON == '}') copy = false;
				pJSON++;
			} while (*pJSON != '\0');
			delay(1);   // give the caller time to receive the data
			brClient.stop();
			inClient.stop();
			digitalWrite(BUSY_PIN, false);
			gotBACK = 1;
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
		Serial.print("Rx: ");
		for (int i = 0; i < buflen; i++)
		{
			Serial.print(((char)buf[i]));
		}

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

			if (wsClient.connect(service_ip, service_port)) {
				wsClient.print("GET /weather/service/data/add?sensor=2&temperature=");
				wsClient.print(temperature);
				wsClient.print("&humidity=");
				wsClient.print(humidity);
				wsClient.println();
				Serial.println(" OK");
				gotACK = 0;
			}
			else
			{
				digitalWrite(ERROR_PIN,true);
				delay(1000);
				digitalWrite(ERROR_PIN,false);
				Serial.println(" ERR");
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

void listenForEthernetClients()
{
	char line[128];
	int nLine = 0;
	
	// listen for incoming clients
	// At the moment I don't look at the incoming request, I just treat all requests as a "Current Weather" query
	if (inClient.connected()) return;
	inClient = server.available();
	if (inClient) {
		Serial.println();
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		boolean was_bridged = false;
		boolean was_foreign = false;
		while (inClient.connected()) {
			if (inClient.available()) {
				char c = inClient.read();
				// got to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank)
				{
					if (was_bridged) return;
					else if (was_foreign)
					{
						inClient.println("HTTP/1.1 500 Internal Server Error");
						inClient.println();
						break;
					}
					else
					{
						Serial.println("Sending back Temp and Humidity");
						// print the current readings, in JSON format:
						inClient.println("HTTP/1.1 200 OK");
						inClient.println("Content-Type: text/json");
						inClient.println();
						inClient.print("{\"Temperature\":\"");
						inClient.print(temperature);
						inClient.print("\",\"Humidity\":\"");
						inClient.print(humidity);
						inClient.println("\"}");
						break;
					}
				}
				if (c == '\n') {
					// Starting a new line
					// Process the line that just finished if appropriate
					if (strstr(line,"Host") == line)
					{
						line[nLine]= '\0';
						Serial.println(line);
						was_foreign = strstr(line,"xyzzy.gordonknight.co.uk") == 0;
					}
					else if (strstr(line,"GET") == line && strstr(line,"HTTP/1.1") == line+nLine-8) {
						// We might want to bridge this command
						line[nLine]= '\0';
						Serial.println(line);
						line[nLine-9]='\0';
						was_bridged = bridged(line+4);
					}
					nLine = 0;
					currentLineIsBlank = true;
				} 
				else if (c != '\r') {
					// Got a character on the current line
					line[nLine++] = c;
					currentLineIsBlank = false;
				}
			}
    	}
    	delay(1);			// give the caller time to receive the data
    	inClient.stop();	// close the connection:
	}
}

boolean bridged(char *request)
{
	// Look at the request and if appropriate make a further request to the bridge server
	// So this is a sort of proxy or bridged request
	if (strstr(request,"/weather/") == request) {
		request = request+9;
		if (strstr(request,"current.json") == request)
		{
			if (brClient.connect(station_ip, station_port)) {
				digitalWrite(BUSY_PIN, true); // Flash a light to show bridging command
				brClient.println("GET http://192.168.0.41:8040/weather/current.json HTTP/1.0");
				brClient.println();
				nJSON = 0;
				gotBACK = 0;
				return true;
			}
		}
	}
	digitalWrite(ERROR_PIN, true); // Flash a light to show error
	delay(1000);
	digitalWrite(ERROR_PIN, false);
	return false;
}
