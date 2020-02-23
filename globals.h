#define USE_PROGMEMCRC 1
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

const byte CC1101_CS = 10;  // Define the Chip Select pin
const byte RXPin = 2;
const byte CDPin = 9;       // wlowi: carrier detect pin
const byte DEBUGPIN = 6;

volatile static unsigned long LastEdgeTime_us = 0;

volatile byte Timings[256];
volatile byte TimingsIndex = 0;

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
