#include <Wire.h>
#include "IDLookup.h"

/* Receive state machine:
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

static volatile bool FirstEdgeState = LOW;

typedef struct statistics_t {
  unsigned long cs_interrupts;
  unsigned long data_interrupts;
  unsigned long carrier_detected;
  unsigned long data_available;
  unsigned int carrier_len;
  unsigned int max_timings;
  unsigned int preamble_found;
  unsigned int checksum_ok;
  unsigned int checksum_fails;
} statistics_t;

static volatile statistics_t statistics;

/* Note: dump happens unlatched. 
 * Wrong data may be printed (sometimes).
 */
void dump_statistics()
{
  Serial.print(F("cs interrupts   : "));
  Serial.println(statistics.cs_interrupts);
  Serial.print(F("data interrupts : "));
  Serial.println(statistics.data_interrupts);
  Serial.print(F("last carrier us : "));
  Serial.println(statistics.carrier_len);  
  Serial.print(F("carrier detected: "));
  Serial.println(statistics.carrier_detected);
  Serial.print(F("data available  : "));
  Serial.println(statistics.data_available);
  Serial.print(F("max timings     : "));
  Serial.println(statistics.max_timings);
  Serial.print(F("preamble found  : "));
  Serial.println(statistics.preamble_found);
  Serial.print(F("checksum ok     : "));
  Serial.println(statistics.checksum_ok);
  Serial.print(F("checksum failed : "));
  Serial.println(statistics.checksum_fails);
}

void clear_statistics()
{
  memset( &statistics, 0, sizeof(statistics));
}
  
/**************************************/

void ClearTPMSData(int i)
{
  if (i > 4)
    return;

  TPMS[i].TPMS_ID = 0;
  TPMS[i].lastupdated = 0;

}

int GetPreferredIndex(unsigned long ID)
{
  int i;

  for (i = 0; i  < (sizeof(IDLookup) / sizeof(IDLookup[0])); i++)
  {
    if (IDLookup[i] == ID)
    {
      return (i);
    }

  }
  return (-1);
}

void InitTPMS()
{
  int i;

  for (i = 0; i < 4; i++)
  {
    ClearTPMSData(i);
  }

  UpdateDisplay();

}

void UpdateTPMSData(int index, unsigned long ID, unsigned int status, float Temperature, float Pressure)
{

  if (index >= 4)
    return;

  TPMS[index].TPMS_ID = ID;
  TPMS[index].TPMS_Status = status;
  TPMS[index].lastupdated = millis();
  TPMS[index].TPMS_Temperature = Temperature;
  TPMS[index].TPMS_Pressure = Pressure;
}

boolean Check_TPMS_Timeouts()
{
   byte i;
   boolean ret = false;
    
    //clear any data not updated in the last 5 minutes
  for (i = 0; i < 4; i++)
  {

    if ((TPMS[i].TPMS_ID != 0) && (millis() - TPMS[i].lastupdated > TPMS_TIMEOUT))
    {
      #ifdef SHOWDEBUGINFO
         Serial.println(F("Clearing ID "));
         Serial.println(TPMS[i].TPMS_ID, HEX);
      #endif
      ClearTPMSData(i);
      ret = true;
    }

  }
  return(ret);
}

// ********************************************  interrupt handler   *******************************

void EdgeInterrupt()
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
    
      FirstEdgeState = digitalRead(RXPin);
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

ISR( PCINT0_vect)
// void CarrierSenseInterrupt()
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
        statistics.carrier_len = CD_Width = ts - CD_Width;
        receiver_state = STATE_IDLE;
      }
      break;

    case STATE_RECEIVING:
      if( carrier == LOW) {
        statistics.carrier_len = CD_Width = ts - CD_Width;
        receiver_state = STATE_DATA_AVAILABLE;
        statistics.data_available++;
      }
      break;

    case STATE_DATA_AVAILABLE:
      /* DO nothing */
      break;      
  }
}

// ***********************************  end of interrupt handler   *******************************

void InitDataBuffer()
{
  TimingsIndex = 0;
}

void UpdateStatusInfo()
{
  FreqOffset = readStatusReg(CC1101_FREQEST);
  DemodLinkQuality = readStatusReg(CC1101_LQI);
  RSSIvalue = readStatusReg(CC1101_RSSI);
}
