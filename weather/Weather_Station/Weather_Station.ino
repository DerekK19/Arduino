
// include the library code:
#include <WeatherStation.h>

void setup()
{
  Serial.begin(9600);
  Serial.println();
  Serial.println ("\n\n[Weather station test program]");
  startWirelessWeather();
}

void loop()
{
  getWirelessWeather();
}
