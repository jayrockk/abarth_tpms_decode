
#include "IDLookup.h"

struct TPMS_entry
{
  unsigned long TPMS_ID;
  unsigned long lastupdated;
  unsigned int TPMS_Status;
  float TPMS_Pressure;
  float TPMS_Temperature;
} TPMS[4];

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

void StoreTPMSData(unsigned long id, unsigned int status, float temperature, float pressure)
{
  bool IDFound = false;  
  int prefindex;
  byte i;
  
  //update the array of tyres data
  for (i = 0; i < 4; i++)
  { //find a matching ID if it already exists
    if (id == TPMS[i].TPMS_ID)
    {
      UpdateTPMSData(i, id, status, temperature, pressure);
      IDFound = true;
      break;
    }
  }

  //no matching IDs in the array, so see if there is an empty slot to add it into, otherwise, ignore it.
  if (IDFound == false)
  {
    prefindex = GetPreferredIndex(id);
    if (prefindex == -1)
    { //not found a specified index, so use the next available one..
      for (i = 0; i < 4; i++)
      {
        if (TPMS[i].TPMS_ID == 0)
        {
#ifdef SHOWDEBUGINFO
          Serial.println(F("No match"));
#endif
          UpdateTPMSData(i, id, status, temperature, pressure);
          break;
        }
      }
    }
    else
    { //found a match in the known ID list...
#ifdef SHOWDEBUGINFO
      Serial.println(F("New match"));
#endif
      UpdateTPMSData(prefindex, id, status, temperature, pressure);
    }
  }
}

bool Check_TPMS_Timeouts()
{
   byte i;
   bool ret = false;
    
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
