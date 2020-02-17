#include <Wire.h>

void ClearTPMSData(int i)
{
  if (i > 4)
    return;

  TPMS[i].TPMS_ID = 0;
  TPMS[i].lastupdated = 0;

}

void PulseDebugPin(int width_us)
{
  digitalWrite(DEBUGPIN, HIGH);
  delayMicroseconds(width_us);
  digitalWrite(DEBUGPIN, LOW);
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

void PrintBits(byte StartPoint, byte Count)
{
  byte i;
  for (i = 0; i < Count; i++)
  {
    Serial.println(IncomingBits[StartPoint + i]);
    Serial.println(F(","));
  }
  Serial.println(F(""));
}
void PrintTimings(byte StartPoint, byte Count)
{
  byte i;
  for (i = 0; i < Count; i++)
  {
    Serial.println(Timings[StartPoint + i]);
    Serial.println(F(","));
  }
  Serial.println(F(""));
}

void PrintData(byte Count)
{
  byte i;
  byte hexdata;
  for (i = 0; i < Count; i++)
  {
    Serial.println(IncomingBits[i]);
    hexdata = (hexdata << 1) + IncomingBits[i];
    if ((i + 1) % 8 == 0)
    {
      Serial.println(F(" ["));
      Serial.println(hexdata, HEX);
      Serial.println(F("] "));
      hexdata = 0;
    }
  }
  Serial.println(F(""));
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

void DisplayStatusInfo()
{
  Serial.println (F("FreqOffset: "));
  Serial.println (FreqOffset);
  Serial.println (F("  DemodLinkQuality: "));
  Serial.println (DemodLinkQuality);
  Serial.println (F("  RSSI: "));
  Serial.println (RSSIvalue);
}

boolean Check_TPMS_Timeouts()
{
   byte i;
   boolean ret = false;
    
    //clear any data not updated in the last 5 minutes
  for (i = 0; i < 4; i++)
  {
    #ifdef SHOWDEGUGINFO
      if((TPMS[i].TPMS_ID) !=0)  //added by jarock
      {                                //added by jarock
        Serial.println(TPMS[i].TPMS_ID, HEX);
        Serial.println(F("   "));
      }                            //added by jarock
    #endif

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

void ClearRXBuffer()
{
  int i;

  for (i = 0; i < sizeof(RXBytes); i++)
  {
    RXBytes[i] = 0;
  }
}

//********************************************  interrupt handler   *******************************
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

//********************************************  end of interrupt handler   *******************************

bool IsTooShort(byte Width)
{
  if (Width < 35)
  {
    return (true);
  }
  else
  {
    return (false);
  }
}

bool IsTooLong(byte Width)
{
  if (Width > 120)
  {
    return (true);
  }
  else
  {
    return (false);
  }
}

bool IsValidSync(byte Width)
{
  if (Width >=  175)
  {
    return (true);
  }
  else
  {
    return (false);
  }
}

bool IsValidShort(byte Width)
{
  if ((Width >= 35) && (Width <= 68))
  {
    return (true);
  }
  else
  {
    return (false);
  }
}


bool IsValidLong(byte Width)
{
  if ((Width >= 80) && (Width <= 120))
  {
    return (true);
  }
  else
  {
    return (false);
  }
}

int ValidateBit()
{
  byte BitWidth = Timings[CheckIndex];
  byte BitWidthNext = Timings[CheckIndex + 1];

  if (IsValidLong(BitWidth))
  {
    return (1);
  }

  if (IsValidShort(BitWidth))
  {
    return (0);
  }

  if (IsValidSync(BitWidth))
  {
    return (2);
  }


  return (-1);

}

static inline uint8_t bit_at(const uint8_t *bytes, unsigned bit)

{
    return (uint8_t)(bytes[bit >> 3] >> (7 - (bit & 7)) & 1);
}

int FindManchesterStart()
{
  int i;


  //Renault TMS header pattern
   const uint8_t pattern[] = {0xAA, 0xA9};
   int pattern_bits_len = 16;
   unsigned int ipos = 0;
   unsigned int ppos = 0; // cursor on init pattern

    while ((ipos < BitCount-3) && (ppos < pattern_bits_len)) 
    {
        if (IncomingBits[ipos] == bit_at(pattern, ppos)) 
        {
            ppos++;
            ipos++;
            if (ppos == pattern_bits_len)
                return ipos;
        }
        else 
        {
            ipos -= ppos;
            ipos++;
            ppos = 0;
        }
    }

    // Not found
    return -1;
 
}

void InvertBitBuffer()
{
   int i;

   for (i = 0;i < BitCount;i++)
   {
      IncomingBits[i] = !IncomingBits[i];
   }
  
}

void ConvertTimingsToBits()
{
   int i;
   
   bool CurrentState = FirstEdgeState;

   BitCount = 0;
   

   for (i=0;i<= TimingsIndex;i++)
   {
      if (IsValidShort(Timings[i]) )
      {
         IncomingBits[BitCount++] = CurrentState;
      }
      if (IsValidLong(Timings[i]) )
      {
         IncomingBits[BitCount++] = CurrentState;
         IncomingBits[BitCount++] = CurrentState;
      }
      if (IsTooShort(Timings[i] ) || IsTooLong(Timings[i] ) )
      {
         if (BitCount == 0)
         { //invalid bit timing, ignore if at start of data stream
             
         }
         else
         {// end the conversion
            return;
         }
      }

      CurrentState = !CurrentState;

      if (BitCount >= MAXBITS-1) 
      {
         return;
      }
   }
  
}

int ManchesterDecode(int StartIndex)
{
   int i;
   bool bit1, bit2;
   byte b = 0;
   byte n = 0;

   RXByteCount = 0;
   for (i = StartIndex; i< BitCount-1;i+=2)
   {
      bit1 = IncomingBits[i];
      bit2 = IncomingBits[i+1];

      if (bit1 == bit2)
         return RXByteCount;

    b = b << 1;
    b = b + (bit2 == true? 1:0);
    n++;
    if (n == 8)
    {
      RXBytes[RXByteCount] = b;
      RXByteCount++;
      n = 0;
      b = 0;
    }     
    
   }

   return RXByteCount;

}


int ValidateTimings()
{


  byte BitWidth;
  byte BitWidthNext;
  byte BitWidthNextPlus1;
  byte BitWidthPrevious;
  byte diff = TimingsIndex - CheckIndex;
  //unsigned long tmp;
  bool WaitingTrailingZeroEdge = false;
  int ret;
  int ManchesterStartPos = -1;
  byte bcount = 0;

  StartDataIndex = 0;

  if (TimingsIndex < 16 + 72)  //header + valid data (minimum)
  { //not enough in the buffer to consider a valid message
    Serial.println(F("Insufficient data in buffer"));
    return -1;
  }

  if (TimingsIndex > 200)  //header + valid data (minimum)
  { //not enough in the buffer to consider a valid message
    Serial.println(F("Excessive data in buffer"));
    return -1;
  }

  //Serial.println("Timings index = ");
  //Serial.println(TimingsIndex);

  ConvertTimingsToBits();

  //InvertBitBuffer();


  ManchesterStartPos = FindManchesterStart();
  Serial.println(ManchesterStartPos);
  if (ManchesterStartPos == -1 )
  {
    Serial.println("Renault header not found");

    return -1;    
  }
  else
  {
     #ifdef SHOWDEGUGINFO
       Serial.println("Timings index = ");
       Serial.println(TimingsIndex);
       Serial.println("CD Width = ");
       Serial.println(CD_Width);
       Serial.println("Bit count = ");
       Serial.println(BitCount);
       PrintTimings(0,TimingsIndex);
       PrintBits(0,BitCount);
    #endif 
  }

  bcount = ManchesterDecode(ManchesterStartPos);

     #ifdef SHOWDEGUGINFO      //jayrock
       Serial.println("bcount = ");//jayrock
       Serial.println(bcount);//jayrock
    #endif 
  
  if (bcount == 9)
  {
     return 9;
  }
  else
  {
    return -1;
  }
}



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

int ReceiveMessage()
{

  //Check bytes in FIFO
  int FIFOcount;
  int resp;
  int ValidRenault = -1;

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
    digitalWrite(LED_RX,LED_ON);
    CheckIndex = 0;
    digitalWrite(LED_RX,LED_OFF);
    ValidRenault = 1;
    return ValidRenault;
  }
  else
  {
    return(-1);
  }
}
