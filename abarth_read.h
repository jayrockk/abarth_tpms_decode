#include <Wire.h>

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


static volatile unsigned int carrierDetected_count;
static volatile unsigned int dataAvailable_count;

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
      #ifdef SHOWDEGUGINFO
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

void CarrierSenseInterrupt()
{
  bool carrier = digitalRead(CDPin);

  switch( receiver_state)
  {
    case STATE_IDLE:
      if( carrier == HIGH) {
        CD_Width = LastEdgeTime_us = micros();
        receiver_state = STATE_CARRIER_DETECTED;
        carrierDetected_count++;
      }
      break;

    case STATE_CARRIER_DETECTED:
      if( carrier == LOW) {
        receiver_state = STATE_IDLE;
      }
      break;

    case STATE_RECEIVING:
      if( carrier == LOW) {
        receiver_state = STATE_DATA_AVAILABLE;
        dataAvailable_count++;
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
