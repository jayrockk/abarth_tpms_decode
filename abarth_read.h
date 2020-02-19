#include <Wire.h>

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

bool FirstEdgeState = LOW;

void EdgeInterrupt()
{
  unsigned long ts = micros();
  unsigned long BitWidth;
 
  if (TimingsIndex == 0)
  {
     //remember the state of the first entry (all other entries will assume to be toggled from this state)
     FirstEdgeState = digitalRead(RXPin);
  }

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
}

// ***********************************  end of interrupt handler   *******************************

void InitDataBuffer()
{
  BitIndex = 0;
  BitCount = 0;
  ValidBlock = false;
  WaitingFirstEdge  = true;
  CheckIndex = 0;
  TimingsIndex = 0;
  SyncFound = false;
  //digitalWrite(DEBUGPIN, LOW);
}

void UpdateStatusInfo()
{
  FreqOffset = readStatusReg(CC1101_FREQEST);
  DemodLinkQuality = readStatusReg(CC1101_LQI);
  RSSIvalue = readStatusReg(CC1101_RSSI);
}

bool ReceiveMessage()
{
  InitDataBuffer();
  
  //set up timing of edges using interrupts...
  LastEdgeTime_us = micros();
  CD_Width = micros();

  attachInterrupt(digitalPinToInterrupt(RXPin), EdgeInterrupt, CHANGE);
  while (GetCarrierStatus() == true)
  {
  }
  detachInterrupt(digitalPinToInterrupt(RXPin));

  CD_Width = micros() - CD_Width;
  if ((CD_Width >= 9500) && (CD_Width <= 10500))//jayrock set upper border to 10500
  {
    return true;
  }
  else
  {
    return false;
  }
}
