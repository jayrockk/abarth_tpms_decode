/*
 * Receive data from CC1101.
 * This version of the code is interrupt driven.
 * 
 * 
 * Receive state machine:
 * ======================
 * 
 *     +------------ CDintr ---------+
 *     |         (carrier lost)      |
 *     V                             |
 *   IDLE >------ CDintr ----> CARRIER_DETECTED
 *     ^    (carrier detected)       |
 *     |                             |
 *     |                             |
 *   loop()                       EdgeIntr
 *  processing                       |
 *     |                             |
 *     |                             V
 *   DATA <------ CDintr ------< RECEIVING >--+
 * AVAILABLE   (carrier lost)        ^        |
 *                                   |     EdgeIntr
 *                                   |        |
 *                                   +--------+
 * 
 */
#define STATE_IDLE               0
#define STATE_CARRIER_DETECTED   1
#define STATE_RECEIVING          2
#define STATE_DATA_AVAILABLE     3

static volatile byte receiver_state;

static volatile bool first_edge_state = LOW;

volatile byte Timings[256];
volatile byte TimingsIndex = 0;

volatile static unsigned long LastEdgeTime_us = 0;

unsigned long CD_Width;

void InitDataBuffer()
{
  TimingsIndex = 0;
}

// ********************************************  interrupt handler   *******************************

#ifndef UNITTEST

void edge_interrupt()
{
  unsigned long ts = micros();
  unsigned long BitWidth;

  statistics.data_interrupts++;
  
  switch( receiver_state)
  {
    case STATE_IDLE:
      /* Do nothing */
      break;

    case STATE_CARRIER_DETECTED:
    
      first_edge_state = digitalRead(RXPin);
      receiver_state = STATE_RECEIVING;
      /* Fall throught */

    case STATE_RECEIVING:

      if (TimingsIndex == 255)
      {//buffer full - don't accpet anymore
        return;
      }

      BitWidth = ts - LastEdgeTime_us;
      LastEdgeTime_us = ts;

      if (BitWidth <= 12)  //ignore glitches
      {
        return;
      }
  
      if (BitWidth > 255)
        BitWidth = 255;

      Timings[TimingsIndex++] = (byte)BitWidth;
      
      break;

    case STATE_DATA_AVAILABLE:
      /* DO nothing */
      break;      
  }
}

void carrier_sense_interrupt()
{
  unsigned long ts = micros();
  byte carrier = digitalRead(CDPin);
  
  statistics.cs_interrupts++;

  switch( receiver_state)
  {
    case STATE_IDLE:
      if( carrier == HIGH) {
        CD_Width = LastEdgeTime_us = ts;
        receiver_state = STATE_CARRIER_DETECTED;
        statistics.carrier_detected++;
      }
      break;

    case STATE_CARRIER_DETECTED:
      if( carrier == LOW) {
        CD_Width = ts - CD_Width;
        if( CD_Width > statistics.carrier_len) {
          statistics.carrier_len = CD_Width;
        }
        receiver_state = STATE_IDLE;
      }
      break;

    case STATE_RECEIVING:
      if( carrier == LOW) {
        CD_Width = ts - CD_Width;
        if( CD_Width > statistics.carrier_len) {
          statistics.carrier_len = CD_Width;
        }        
        receiver_state = STATE_DATA_AVAILABLE;
        statistics.data_available++;
      }
      break;

    case STATE_DATA_AVAILABLE:
      /* DO nothing */
      break;      
  }
}

#endif

// ***********************************  end of interrupt handler   *******************************
