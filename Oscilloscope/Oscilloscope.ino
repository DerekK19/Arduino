#include "stdio.h"
#include "pins_arduino.h"
#include "util/delay.h"

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


uint8_t low1=0, high1=0, low2=0, high2=0;
uint8_t channel=1, prescale=B100;

void setup()
{
    // the fastest throughput I've gotten is ~1.6Mbit...
    //
    // if overflow does happen, it's not really a big deal... we just lose some readings.
    //
    // checking the USART's data-empty register doesn't seem to help at all, I still lose
    //   about 0.05% of my data... I also tried checking the transmit complete register.
    //   so instead of
    //          Serial.write(0xF);
    //   I'm using
    //          UDR0 = 0xF;
    //   which gives me higher throughput with the same amount of errors.
    //
    //
    // Since I'm just accepting that I will lose data in transit I've designed my data
    //   format to be robust and self-correctable. Every 3 bytes has 2 x 10bit
    //   ADC reads and the status of pin 12.
    //       byte[0] = 1, ADC0[9],ADC0[8],ADC0[7],ADC0[6],ADC0[5],ADC0[4],ADC0[3]
    //       byte[1] = 1, ADC1[9],ADC1[8],ADC1[7],ADC1[6],ADC1[5],ADC1[4],ADC1[3]
    //       byte[2] = 0, ADC0[2],ADC0[1],ADC0[0], PIN12 ,ADC1[2],ADC1[1],ADC1[0]
    //
    // If 3 bytes recieved consecutively match this schema, then they are just read. 
    //   If a stray byte is found the highest order bit is checked to see if it
    //   contains a salvagable 7bit ADC reading and the next set of bytes is checked.
    //
    
    int8_t read_val = -1;
    
    // intially set the speed to 1Mbaud and then double it to 2Mbaud with U2X mode
    Serial.begin(1000000);
    UCSR0A |= 2;
    
    // send a message to let the computer know you are alive
    Serial.print("Hello World");
    
    // start up some reference signals
    analogWrite(9,3);
    analogWrite(10,127);
    analogWrite(11,250);
    
    // setup pin 12 for triggers
    pinMode(12, INPUT);
    // if (*portInputRegister(digitalPinToPort(12)) & digitalPinToBitMask(12)) return; // HIGH
    //(*portInputRegister(digitalPinToPort(12)) & digitalPinToBitMask(12)) >> 3; // HIGH input 12
    
    // read in 1 byte (has prescaler setting and channel from computer)
    while(read_val==-1) read_val = Serial.read();
    channel = read_val & 0x7;
    prescale = (read_val >> 3) & 0x7;
    
    // set ADC prescale factor
    _SFR_BYTE(ADCSRA) = (_SFR_BYTE(ADCSRA) & 0xF8) | (0x7 & prescale);
    
    if((prescale==0)||(prescale==1))
    {
        // use the 5V reference, set the channel, and left-adjust the result
        ADMUX = ((DEFAULT << 6) | (channel & 0x0f) | (1 << ADLAR) ) & B11110111;
        
        // start the first conversion
        sbi(ADCSRA, ADSC);
        while(true)
        {
            UDR0 = high2|((high1&0xE0)>>1);
            while (bit_is_set(ADCSRA, ADSC));
            high1 = ADCH;
            sbi(ADCSRA, ADSC); // start an ADC conversion
            high2 = (high1>>4)|((*portInputRegister(digitalPinToPort(12)) & digitalPinToBitMask(12))<<3);
            while (bit_is_set(ADCSRA, ADSC));
            high1 = ADCH;
            sbi(ADCSRA, ADSC); // start an ADC conversion
        }
    }
    else if(prescale==2)
    {
        // use the 5V reference, set the channel, and left-adjust the result
        ADMUX = ((DEFAULT << 6) | (channel & 0x0f) | (1 << ADLAR) ) & B11110111;
        
        // start the first conversion
        sbi(ADCSRA, ADSC);
        while(true)
        {
            UDR0 = (high1>>1)|((*portInputRegister(digitalPinToPort(12)) & digitalPinToBitMask(12))<<3);
            while (bit_is_set(ADCSRA, ADSC));
            high1 = ADCH;
            sbi(ADCSRA, ADSC); // start an ADC conversion
        }
    }
    else
    {
        // use the 5V reference, set the channel, and right-adjust the result
        ADMUX = ((DEFAULT << 6) | (channel & 0x0f) /* | (1 << ADLAR) */ ) & B11110111;
        
        // start the first conversion
        sbi(ADCSRA, ADSC);
        while(true)
        {
            //delay(1); // make it SLOW!! ~1Khz
            //_delay_us(50); // slow it down a bit
            
            //Serial.write((high2 << 5) | (low2 >> 3) | 0x80);
            UDR0 = (high2 << 5) | (low2 >> 3) | 0x80;
            
            while (bit_is_set(ADCSRA, ADSC));
            low1 = ADCL;
            high1 = ADCH;
            sbi(ADCSRA, ADSC); // start an ADC conversion
            
            //Serial.write((high1 << 5) | (low1 >> 3) | 0x80);
            UDR0 = ((high1 << 5) | (low1 >> 3) | 0x80);
        
            //Serial.write(((low1 & 7)<<4)|(low2 & 7)|(*portInputRegister(digitalPinToPort(12)) & digitalPinToBitMask(12)));
            UDR0 = (((low1 & 7)<<4)|(low2 & 7)|(*portInputRegister(digitalPinToPort(12)) & digitalPinToBitMask(12)));
            
            while (bit_is_set(ADCSRA, ADSC));
            low2 = ADCL;
            high2 = ADCH;
            sbi(ADCSRA, ADSC); // start an ADC conversion for the next loop
        }
    }
}

void loop()
{
}
