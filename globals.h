#define SHOWDEBUGINFO 1

#define I2C_ADDRESS 0x3C

#define LED_RX 17
#define LED_OFF HIGH
#define LED_ON LOW

#define TPMS_TIMEOUT 900000 //(15 * 60 * 1000)  15 minutes

#define FONTBAR_7 123
#define FONTBAR_5 124
#define FONTBAR_3 125
#define FONTBAR_2 126
#define FONTBAR_1 127
#define FONTBAR_0 32

const int CC1101_CS = 10;  // Define the Chip Select pin
const int RXPin = 2;
const int CDPin = 9;       // wlowi: carrier detect pin
const int DEBUGPIN = 6;

volatile static unsigned long LastEdgeTime_us = 0;

volatile byte Timings[256];
volatile uint8_t TimingsIndex = 0;

unsigned long CD_Width;

unsigned int FreqOffset;
unsigned int DemodLinkQuality;
unsigned int RSSIvalue;

unsigned long IncomingAddress;


struct TPMS_entry
{
  unsigned long TPMS_ID;
  unsigned long lastupdated;
  unsigned int TPMS_Status;
  float TPMS_Pressure;
  float TPMS_Temperature;
} TPMS[4];


enum RXStates
{
  Waiting_Byte33 = 0,
  Got_Byte33,
  Got_Byte55,
  Got_Byte53,
  Manch1,
  Manch2
};
