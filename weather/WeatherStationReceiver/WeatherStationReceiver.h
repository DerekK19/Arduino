#ifndef WSR_H_
#define WSR_H_

typedef unsigned int uint;
typedef signed int sint;

#define STATE_IDLE                  0
#define STATE_LOADING_BITSTREAM     1

#define BIT_NONE                    0
#define BIT_ZERO                    1
#define BIT_ONE                     2

#define PACKET_RESET() { STATE = STATE_IDLE; bitPointer = 0; }

#define INPUT_CAPTURE_IS_RISING_EDGE()    ((TCCR1B & _BV(ICES1)) != 0)
#define INPUT_CAPTURE_IS_FALLING_EDGE()   ((TCCR1B & _BV(ICES1)) == 0)
#define SET_INPUT_CAPTURE_RISING_EDGE()   (TCCR1B |=  _BV(ICES1))
#define SET_INPUT_CAPTURE_FALLING_EDGE()  (TCCR1B &= ~_BV(ICES1))

#define GREEN_TESTLED_ON()          ((PORTD &= ~(1<<PORTD6)))
#define GREEN_TESTLED_OFF()         ((PORTD |=  (1<<PORTD6)))

#define WSR_RESET()                  {}//  { bICP_WSR_State = WSR_STATE_IDLE; bICP_WSR_PacketInputBitPointer = WSR_TIMESTAMP_BIT_OFFSET; }

#endif