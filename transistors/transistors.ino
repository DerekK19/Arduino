/* An Arduino program must contain two functions, setup and loop */

int counter = 0;

void setup()
{
}

void loop()
{
    if (counter == 0 || counter == 2) digitalWrite(8, LOW);
    if (counter == 0 || counter == 1) digitalWrite(9, LOW);
    if (counter == 1 || counter == 3) digitalWrite(8, HIGH);
    if (counter == 2 || counter == 3) digitalWrite(9, HIGH);
    
    delay (1000);

    counter++;
    if (counter > 3) counter = 0;
}
