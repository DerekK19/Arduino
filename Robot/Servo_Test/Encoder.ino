/*

 Encoder.h - Encoder handling library
 Copyright (c) 2009 Samuel Marco i Armengol.  All rights reserved.
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 The latest version of this library can always be found at
 
 */

#include "pins_arduino.h"
/*
 * an extension to the interrupt support for arduino.
 * add pin change interrupts to the external interrupts, giving a way
 * for users to have interrupts drive off of any pin.
 * Refer to avr-gcc header files, arduino source and atmega datasheet.
 */

/*
 * Theory: all IO pins on Atmega168 are covered by Pin Change Interrupts.
 * The PCINT corresponding to the pin must be enabled and masked, and
 * an ISR routine provided.  Since PCINTs are per port, not per pin, the ISR
 * must use some logic to actually implement a per-pin interrupt service.
 */

/* Pin to interrupt map:
 * D0-D7 = PCINT 16-23 = PCIR2 = PD = PCIE2 = pcmsk2
 * D8-D13 = PCINT 0-5 = PCIR0 = PB = PCIE0 = pcmsk0
 * A0-A5 (D14-D19) = PCINT 8-13 = PCIR1 = PC = PCIE1 = pcmsk1
 */

volatile uint8_t *port_to_pcmask[] = {
  &PCMSK0,
  &PCMSK1,
  &PCMSK2
};

typedef void (*voidFuncPtr)(void);

volatile static voidFuncPtr PCintFunc[24] = { 
  NULL };

volatile static uint8_t PCintLast[3];

/*
 * attach an interrupt to a specific pin using pin change interrupts.
 * First version only supports CHANGE mode.
 */
void PCattachInterrupt(uint8_t pin, void (*userFunc)(void), int mode) {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  uint8_t slot;
  volatile uint8_t *pcmask;

  if (mode != CHANGE) {
    return;
  }
  // map pin to PCIR register
  if (port == NOT_A_PORT) {
    return;
  } 
  else {
    port -= 2;
    pcmask = port_to_pcmask[port];
  }
  slot = port * 8 + (pin % 8);
  PCintFunc[slot] = userFunc;
  // set the mask
  *pcmask |= bit;
  // enable the interrupt
  PCICR |= 0x01 << port;
}

void PCdetachInterrupt(uint8_t pin) {
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *pcmask;

  // map pin to PCIR register
  if (port == NOT_A_PORT) {
    return;
  } 
  else {
    port -= 2;
    pcmask = port_to_pcmask[port];
  }

  // disable the mask.
  *pcmask &= ~bit;
  // if that's the last one, disable the interrupt.
  if (*pcmask == 0) {
    PCICR &= ~(0x01 << port);
  }
}

// common code for isr handler. "port" is the PCINT number.
// there isn't really a good way to back-map ports and masks to pins.
static void PCint(uint8_t port) {
  uint8_t bit;
  uint8_t curr;
  uint8_t mask;
  uint8_t pin;

  // get the pin states for the indicated port.
  curr = *portInputRegister(port+2);
  mask = curr ^ PCintLast[port];
  PCintLast[port] = curr;
  // mask is pins that have changed. screen out non pcint pins.
  if ((mask &= *port_to_pcmask[port]) == 0) {
    return;
  }
  // mask is pcint pins that have changed.
  for (uint8_t i=0; i < 8; i++) {
    bit = 0x01 << i;
    if (bit & mask) {
      pin = port * 8 + i;
      if (PCintFunc[pin] != NULL) {
        PCintFunc[pin]();
      }
    }
  }
}

SIGNAL(PCINT0_vect) {
  PCint(0);
}
SIGNAL(PCINT1_vect) {
  PCint(1);
}
SIGNAL(PCINT2_vect) {
  PCint(2);
}

// End of interrupts code and start of the reader code

class Encoder {
public:
  // private:
  static uint8_t _pA, _pB;
  static int32_t _position;
  static void service(void);
  // public methods
  // public:
  Encoder(uint8_t, uint8_t);
  void start();
  void stop();
  void write(int32_t);
  int32_t read(void);
};

uint8_t Encoder::_pA = 0;
uint8_t Encoder::_pB = 0;
int32_t Encoder::_position = 0;

//
// Constructor
//
Encoder::Encoder(uint8_t pA, uint8_t pB) {
  // initialisation of class attributes
  _pA = pA;
  _pB = pB;
  _position = 0;

  // initialisation of I/O
  pinMode( _pA, INPUT );
  pinMode( _pB, INPUT );
  digitalWrite( _pA, HIGH );  // pull-up
  digitalWrite( _pB, HIGH );  // pull-up
}

void Encoder::start() {
  PCattachInterrupt( _pA, service, CHANGE );
}

void Encoder::stop() {
  PCdetachInterrupt(_pA);
}

void Encoder::write(int32_t position) {
  _position = position;
}

int32_t Encoder::read(void) {
  return _position; 
}

void Encoder::service(void) {
  uint8_t value;
  value = digitalRead(_pA) << 1 + digitalRead(_pB);
  switch (value) {
  case 0:  
  case 3:
    _position--;
    break;
  case 1:
  case 2:
    _position++;
    break;
  }

  if ((digitalRead(_pA) == LOW) ^ (digitalRead(_pB) == HIGH)) _position++;
  else _position--;

}
