/*
 * Constants and global services like statistics maintenance.
 * 
 */

#define USE_PROGMEMCRC 1
#define SHOWDEBUGINFO 1

#define I2C_ADDRESS 0x3C

#define LED_OFF HIGH
#define LED_ON LOW

#define TPMS_TIMEOUT 900000 //(15 * 60 * 1000)  15 minutes

#define FONTBAR_7 123
#define FONTBAR_5 124
#define FONTBAR_3 125
#define FONTBAR_2 126
#define FONTBAR_1 127
#define FONTBAR_0 32

/* Pin assignment */
#define LED_RX    17
#define CC1101_CS 10  // Define the Chip Select pin
#define RXPin      2  // GDO2
#define TXPin      9  // wlowi: GDO0 is also TX pin
#define CDPin      9  // wlowi: GDO0 carrier detect pin
#define DEBUGPIN   6


/***********************************/

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
  Serial.print(F("longest carr. us: "));
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
  memset( (void*)&statistics, 0, sizeof(statistics));
}
