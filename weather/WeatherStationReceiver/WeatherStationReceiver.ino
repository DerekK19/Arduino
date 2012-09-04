#include "weatherstationreceiver.h"

uint captured_time;
uint previous_captured_time;
uint captured_period;
uint previous_captured_period;
byte capture_period_was_high;
byte previous_captured_period_was_high;
unsigned long timestamp_262_144mS;
unsigned long last_timestamp_262_144mS;

byte STATE;

byte packetData[4][5];
uint packetPointer = 0;
uint bitPointer = 0;
uint currentReaderPacket = 1;
uint lastValidPacket = 0;

#define DEBUG

// Any signal shorter than this is noise
#define NOISE_LEVEL_LOW 115
#define NOISE_LEVEL_HIGH 6000

int lo_one = NOISE_LEVEL_LOW;		// Supplied value was 115
int hi_one = 145;		// Supplied value was 145
int lo_zero = 355;		// Supplied value was 355
int hi_zero = NOISE_LEVEL_HIGH;		// Supplied value was 395

int lo_short = NOISE_LEVEL_HIGH;
int hi_short = NOISE_LEVEL_LOW;
int lo_long = NOISE_LEVEL_HIGH;
int hi_long = NOISE_LEVEL_LOW;

void setup(void) {
    Serial.begin(115200);
	Serial.println("\nWeather sensor");
    initi();
    GREEN_TESTLED_OFF();
    interrupts();
}

void initi()
{
    DDRB = 0x2F;   // B00101111
    WSR_RESET();
    DDRB  &= ~(1<<DDB0);
    PORTB &= ~(1<<PORTB0);
    DDRD  |=  B11000000;
    TCCR1A = B00000000;
    TCCR1B = (_BV(ICNC1) | _BV(CS11) | _BV(CS10));
    SET_INPUT_CAPTURE_RISING_EDGE();
    TIMSK1 = (_BV(ICIE1) | _BV(TOIE1));
}

void loop(void)
{
    byte data;
    byte b, c;
    if (currentReaderPacket != lastValidPacket) {
        currentReaderPacket = lastValidPacket;

        GREEN_TESTLED_ON();
        delay(200);
        GREEN_TESTLED_OFF();

        byte crc = getCRC(packetData[currentReaderPacket], 4);
        if (crc != packetData[currentReaderPacket][4]) {
            #ifdef DEBUG
//            Serial.println("BAD CRC!");
            #else
            return;
            #endif
        }
/*
        packetData[currentReaderPacket][2];
        int temp;
        temp = ((packetData[currentReaderPacket][1] & B00000111) << 8) + packetData[currentReaderPacket][2];
        if (packetData[currentReaderPacket][1] & B00001000) {
            temp *= -1;
        }
        Serial.print("TEMP=");
        Serial.println(temp);
        Serial.print("HUMIDITY="); 
        Serial.println(packetData[currentReaderPacket][3], DEC);
*/
//        double intemp = thermister(analogRead(0));
//        Serial.print("INTEMP=");
//        Serial.println(intemp);

        #ifdef DEBUG
        int s = millis()/1000;
        if (s < 10) Serial.print(' ');
        if (s < 100) Serial.print(' ');
        Serial.print(s);
        Serial.print(":  ");
        for (int i = 0; i < 40; ++i) {
            if ((packetData[currentReaderPacket][i >> 3] & (0x80 >> (i & 0x07))) != 0) {
                Serial.print('1');
            } else {
                Serial.print('0');
            }
            if (i == 7 || i == 11) Serial.print(' ');
        }
        Serial.print(' ');
        Serial.print(lo_short);
        Serial.print(' ');
        Serial.print(hi_short);
        Serial.print(' ');
        Serial.print(lo_long);
        Serial.print(' ');
        Serial.print(hi_long);
        Serial.print(' ');
        Serial.println();
		lo_short = NOISE_LEVEL_HIGH;
		hi_short = NOISE_LEVEL_LOW;
		lo_long = NOISE_LEVEL_HIGH;
		hi_long = NOISE_LEVEL_LOW;
        #endif
    }
}

uint8_t getCRC(uint8_t *addr, uint8_t len)
{
    uint8_t crc = 0;
    while (len--) {
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x80;
            crc <<= 1;
            if (mix) crc ^= 0x131;
            inbyte <<= 1;
        }
    }
    return crc;
}

double thermister(int rawADC) {
    double temp;
    temp = log(((10240000/rawADC) - 10000));
    temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp * temp))* temp);
    temp = temp - 273.15;
    return temp;
}


ISR(TIMER1_OVF_vect)
{
    timestamp_262_144mS++;
}

ISR(TIMER1_CAPT_vect)
{
    captured_time = ICR1;
    if (INPUT_CAPTURE_IS_RISING_EDGE()) {
        SET_INPUT_CAPTURE_FALLING_EDGE();
        capture_period_was_high = false;
    } else {
        SET_INPUT_CAPTURE_RISING_EDGE();
        capture_period_was_high = true;
    }

    captured_period = (captured_time - previous_captured_time);
    check_receive_bit();

    previous_captured_time = captured_time;
    previous_captured_period = captured_period;
    previous_captured_period_was_high = capture_period_was_high;
}

void check_receive_bit()
{
    volatile byte b;
    byte is_valid_bit = false; // 0=false(BIT_NONE), 1=BIT_ZERO, 2=BIT_ONE
  
    //discard the captured period if it is out of the expected range, it is noise...
    if ((captured_period >= NOISE_LEVEL_LOW && captured_period <= NOISE_LEVEL_HIGH)) {
//        #ifdef DEBUG
//        Serial.print(capture_period_was_high ? "^: " : "_: ");
//        Serial.println(captured_period);
//        #endif
        if (capture_period_was_high) {
            //got a high period, could be a valid bit
            if ((captured_period >= lo_one) && (captured_period <= hi_one)) {
                //short high, valid one bit
                if (captured_period < lo_short) lo_short = captured_period;
                if (captured_period > hi_short) hi_short = captured_period;
                is_valid_bit = BIT_ONE;
            } else if ((captured_period >= lo_zero) && (captured_period <= hi_zero)) {
                //long high, valid zero bit
                if (captured_period < lo_long) lo_long = captured_period;
                if (captured_period > hi_long) hi_long = captured_period;
                is_valid_bit = BIT_ZERO;
            }
        }
        if (is_valid_bit != false) {
            switch (STATE) {
                case STATE_IDLE: {
                    if (is_valid_bit == BIT_ZERO) {
                        packetData[packetPointer][bitPointer >> 3] &= ~(0x80 >> (bitPointer & 0x07));
                        bitPointer++;
                        STATE = STATE_LOADING_BITSTREAM;
                    } else {
                        PACKET_RESET();
                    }
                    break;
                }
                case STATE_LOADING_BITSTREAM: {
                    // Potentially valid packet bitstream is on its way in, keep loading it up
                    if (is_valid_bit == BIT_ZERO) {
                        packetData[packetPointer][bitPointer >> 3] &= ~(0x80 >> (bitPointer & 0x07));
                    } else {
                        packetData[packetPointer][bitPointer >> 3] |=  (0x80 >> (bitPointer & 0x07));
                    }

                    if (bitPointer == 3) {
                        //check if first 4 bits are 0101
                        b = packetData[packetPointer][0];
                        b &= B11110000;
                        if (b != B01010000) {
                            PACKET_RESET();
                            break;
                        }
                    }

                    if (bitPointer == 7) {
                        //check if first 8 bits are 01010101
                        b = packetData[packetPointer][0];
                        b &= B11111111;
                        if (b != B01010101) {
                            PACKET_RESET();
                            break;
                        }
                    }

                    if (bitPointer == 40) {
                        // Got full packet
                        lastValidPacket = packetPointer;
                        packetPointer++;
                        if (packetPointer == 4) {
                            packetPointer = 0;
                        }
                        PACKET_RESET();
                        break;
                    }
                    bitPointer++;
                    break;
                }
            }
        }
    } else {
        //PACKET_RESET();
    }
}